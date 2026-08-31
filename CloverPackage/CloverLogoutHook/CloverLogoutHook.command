#!/bin/bash
# =====================================================================
#  CloverLogoutHook
#  Persistent NVRAM for Clover Legacy Boot.
#  Copyright © 2026 chris1111, All Rights Reserved.
#
#  WHAT IT DOES
#    • Keeps <CloverVolume>/nvram.plist always in sync with live NVRAM.
#    • Sticky anchor (sticky.xml): keys macOS consumes after boot
#      (efi-boot-device*) are NEVER lost — they travel to every boot.
#    • LIVE-WINS merge (PlistBuddy): fresh Startup Disk values always
#      overwrite old ones; the anchor only re-adds consumed keys.
#    • Boot-args control: manage boot-args from the terminal without
#      touching config.plist (setargs command). Stored as native EFI
#      binary form (<data>), the form the kernel consumes.
#    • Self-heal: Clover file rebuilt at daemon start + verified every
#      minute against the anchor.
#    • Invisible: transient hidden mount (nobrowse), released after
#      use; user mounts are reused, never stolen.
#    • Housekeeping: log rotation, tmp cleanup, 'clean' command.
#
#  NEW IN v1.6.2 — LEGACY macOS MOUNT SUPPORT (10.6 - 10.8)
#    • mount_esp(): 4-method cascade — modern diskutil, diskutil
#      simple, legacy mount_msdos/mount_hfs into the hidden folder,
#      and a visible last-resort. Covers systems where
#      "-mountPoint" does not exist (Snow Leopard, Lion, Mountain
#      Lion). Verified on 10.6.8.
#    • release_esp(): unmount by disk identifier (more reliable
#      than mount point on legacy systems).
#    • v1.6.1 fix re-included: setargs creates a minimal anchor on
#      fresh rigs (no Startup Disk click ever required).
#
#  FROM v1.6.1 / v1.6
#    • Native Foundation data codec for boot-args (no manual
#      base64/hex chains, no PlistBuddy data quirks, no perl).
#    • Round-trip verification before any Clover-file write.
#    • EXPERIMENT-PROVEN: Clover MERGES config arguments with the
#      NVRAM-file arguments at boot.
#    • HFS+ boot support, multi-disk internal discovery.
#    • /Library/Logs/CloverHook, full uninstall cleanup.
#
#  LAYOUT OF THIS FILE
#    §1  Configuration            §7  Core sync (sync_once)
#    §2  Output & logging         §8  Clover file verification
#    §3  Generic helpers          §9  Housekeeping
#    §4  ESP/Clover resolution    §10 Daemon (watch / install / remove)
#    §5  Diagnostics              §11 Maintenance (clean / setargs / prune)
#    §6  Boot-args data codec     §12 Command dispatcher
# =====================================================================

VERSION="1.6.2"
set -uo pipefail


# ────────────────────────────────────────────────────────────────────────
#  §1  CONFIGURATION
# ────────────────────────────────────────────────────────────────────────

# --- Daemon identity ----------------------------------------------------
DAEMON_LABEL="com.clover.nvramhook.daemon"
INSTALL_BIN="/usr/local/bin/clover-logout-hook"
DAEMON_PLIST="/Library/LaunchDaemons/${DAEMON_LABEL}.plist"

# --- Timing -------------------------------------------------------------
POLL_INTERVAL="${CLOVER_POLL_INTERVAL:-2}"   # cheap check cadence (sec)
VERIFY_EVERY=30                              # verify+rotate every N cycles

# --- Paths --------------------------------------------------------------
LOG_DIR="/Library/Logs/CloverHook"
LOG_FILE="$LOG_DIR/nvramhook.log"
ESP_CACHE="$LOG_DIR/last-esp.txt"            # cached Clover-volume identifier
LAST_XML="$LOG_DIR/last.xml"                 # raw live baseline (change detect)
STICKY_XML="$LOG_DIR/sticky.xml"             # anchor: survives macOS consumption
HIDDEN_MP="/private/var/.CloverEFI"          # transient mount point (invisible)

# --- Boot-selection keys kept alive across boots -------------------------
HEAL_KEYS=(
    "efi-boot-device"
    "efi-boot-device-data"
    "efi-apple-recovery"
    "boot-args"                                # v1.5: sticky-only key
)

# --- Keys EXCLUDED from live-wins merge (anchor is the sole owner) -------
#     boot-args: Clover reads them from NVRAM after importing the file,
#     so the anchor must be the single source of truth.
MERGE_EXCLUDE=(
    "boot-args"
)

# --- Internal state -----------------------------------------------------
ESPID=""                                     # current Clover-volume identifier
HOST_DISK=""                                 # physical disk hosting the OS
MP=""                                        # current volume mount point
OWN_MOUNT=0                                  # 1 = we created this mount


# ────────────────────────────────────────────────────────────────────────
#  §2  OUTPUT & LOGGING
# ────────────────────────────────────────────────────────────────────────

setup_log_dir() {
    if [[ -d "/Library/Logs/CloverHook" ]]; then
        LOG_DIR="/Library/Logs/CloverHook"
    elif mkdir -p "/Library/Logs/CloverHook" 2>/dev/null \
         && [[ -w "/Library/Logs/CloverHook" ]]; then
        LOG_DIR="/Library/Logs/CloverHook"
    else
        LOG_DIR="/tmp/clover-nvramhook"
        mkdir -p "$LOG_DIR" 2>/dev/null
    fi
}

log()  { echo "$(date '+%Y-%m-%d %H:%M:%S') [v${VERSION}] $*" >> "$LOG_FILE" 2>/dev/null; }
dbg()  { log "DEBUG: $*"; }
die()  { echo "ERROR: $*" >&2; log "ERROR: $*"; exit 1; }
info() { echo "INFO:  $*"; }
ok()   { echo "OK:    $*"; }
warn() { echo "WARN:  $*"; }

banner() {
    echo "CloverLogoutHook v${VERSION}"
}


# ────────────────────────────────────────────────────────────────────────
#  §3  GENERIC HELPERS
# ────────────────────────────────────────────────────────────────────────

need_root() {
    [[ $(id -u) -eq 0 ]] || die "This action needs sudo:  sudo $0 ${1:-}"
}

is_mounted_at() {
    mount | grep -qF " on $1 "
}

# Prints the mount point of a partition identifier (space-safe), or fails.
mounted_mp_of_espid() {
    local mp
    [[ -n "$ESPID" ]] || return 1
    mp="$(diskutil info "$ESPID" 2>/dev/null \
          | awk -F': *' '/^ *Mount Point:/ {print $2; exit}')"
    if [[ "$mp" == /* ]]; then
        printf '%s\n' "$mp"
        return 0
    fi
    return 1
}


# ────────────────────────────────────────────────────────────────────────
#  §4  ESP / CLOVER VOLUME RESOLUTION
#
#      APFS path:  "/" -> snapshot -> container -> physical store -> disk
#      HFS path:   "/" -> the HFS disk itself
#
#      Then:  EFI-typed partition (boot disk) -> internal HFS volumes
#             hosting EFI/CLOVER (multi-disk, boot disk first).
# ────────────────────────────────────────────────────────────────────────

get_root_slice() {
    mount | awk '$3=="/" {sub("^/dev/","",$1); print $1; exit}'
}

get_disk_prefix() {                        # "disk5s5s1" -> "disk5"
    [[ $1 =~ ^(disk[0-9]+) ]] && echo "${BASH_REMATCH[1]}"
}

get_physical_store() {                     # APFS only: "disk5" -> "disk3s2"
    diskutil list "$1" 2>/dev/null \
        | awk '/Physical Store[ \t]/ {print $NF; exit}'
}

get_efi_partition() {                      # "disk3" -> "disk3s1" (or "")
    diskutil list "$1" 2>/dev/null \
        | awk '/^[ \t]*[0-9]+:/ && /EFI/ {print $NF; exit}'
}

# probe_clover_volume <diskXsY>
#   Succeeds if the volume hosts an EFI/CLOVER folder.
#   On APFS boots: the live root is a sealed snapshot - skip its volume.
#   On HFS boots:  the live root is writable and MAY host EFI/CLOVER.
probe_clover_volume() {
    local part="$1"
    local root_slice mp own=0 found=0

    root_slice="$(get_root_slice)"

    if [[ $root_slice =~ ^disk[0-9]+s[0-9]+s[0-9]+$ ]]; then
        local root_vol=""
        [[ $root_slice =~ ^(disk[0-9]+s[0-9]+) ]] && root_vol="${BASH_REMATCH[1]}"
        if [[ "$part" == "$root_vol" ]]; then
            dbg "probe: skip sealed APFS system volume $part"
            return 1
        fi
    fi

    mp="$(diskutil info "$part" 2>/dev/null \
          | awk -F': *' '/^ *Mount Point:/ {print $2; exit}')"
    if [[ "$mp" == /* ]]; then
        [[ -d "$mp/EFI/CLOVER" ]] && return 0
        return 1
    fi

    # transient hidden mount for the probe
    mkdir -p "$HIDDEN_MP" 2>/dev/null
    if ! diskutil mount -mountOptions nobrowse -mountPoint "$HIDDEN_MP" "$part" >/dev/null 2>&1 \
       && ! diskutil mount -mountPoint "$HIDDEN_MP" "$part" >/dev/null 2>&1; then
        dbg "probe: cannot mount $part"
        return 1
    fi
    own=1
    mp="$HIDDEN_MP"

    [[ -d "$mp/EFI/CLOVER" ]] && found=1

    if (( own )); then
        diskutil unmount "$mp"      >/dev/null 2>&1 \
            || diskutil unmount force "$mp" >/dev/null 2>&1
    fi

    (( found )) && return 0 || return 1
}

# discover_clover_volume [host-whole-disk]
#   Scans ALL INTERNAL disks for a volume hosting EFI/CLOVER,
#   boot disk first (its result wins). External/USB disks excluded.
discover_clover_volume() {
    local host_disk="${1:-}"
    local dev part

    dbg "discover: scanning INTERNAL disks for EFI/CLOVER volumes…"

    local -a candidates=()
    [[ -n "$host_disk" ]] && candidates+=("$host_disk")

    while read -r dev; do
        [[ "$dev" == "$host_disk" ]] && continue
        if diskutil info "$dev" 2>/dev/null | grep -q "Internal.*Yes"; then
            candidates+=("$dev")
        fi
    done < <(diskutil list 2>/dev/null \
             | awk '/^\/dev\/disk[0-9]+ \(/ {gsub("/dev/","",$1); gsub(":","",$1); print $1}')

    (( ${#candidates[@]} > 0 )) || return 1

    local candidate
    for candidate in "${candidates[@]}"; do
        for part in $(diskutil list "$candidate" 2>/dev/null \
                      | awk '/Apple_HFS/ {print $NF}'); do
            dbg "discover: probing $part (disk $candidate)…"
            if probe_clover_volume "$part"; then
                ESPID="$part"
                printf '%s\n' "$ESPID" > "$ESP_CACHE" 2>/dev/null
                log "discover: Clover volume found at $part (HFS+, disk $candidate)"
                return 0
            fi
        done
    done

    dbg "discover: no internal HFS+ volume hosts EFI/CLOVER"
    return 1
}

resolve_boot_esp_id() {
    local slice container store whole espid

    slice="$(get_root_slice)"
    dbg "resolve: root slice     = ${slice:-MISSING}"
    [[ -n "$slice" ]] || return 1

    container="$(get_disk_prefix "$slice")"
    dbg "resolve: boot disk      = ${container:-MISSING}"
    [[ -n "$container" ]] || return 1

    store="$(get_physical_store "$container")"

    if [[ "$store" =~ ^disk[0-9]+s[0-9]+$ ]]; then
        # APFS boot: physical store -> real host disk
        whole="$(get_disk_prefix "$store")"
        dbg "resolve: apfs store     = $store"
        dbg "resolve: host whole     = ${whole:-MISSING}"
    else
        # HFS boot: no APFS layer, the boot disk hosts the root
        whole="$container"
        dbg "resolve: HFS boot (no APFS store) - host whole = $whole"
    fi
    [[ -n "$whole" ]] || return 1
    HOST_DISK="$whole"

    # Priority 1 - EFI-typed partition on the boot disk (GPT ESP)
    espid="$(get_efi_partition "$whole")"
    if [[ "$espid" =~ ^disk[0-9]+s[0-9]+$ ]]; then
        ESPID="$espid"
        printf '%s\n' "$ESPID" > "$ESP_CACHE" 2>/dev/null
        dbg "resolve: clover volume (EFI-typed) = $espid"
        return 0
    fi

    # Priority 2 - internal HFS+ volume hosting EFI/CLOVER (any disk)
    dbg "resolve: no EFI-typed partition on boot disk - multi-disk scan…"
    discover_clover_volume "$whole"
}


# ────────────────────────────────────────────────────────────────────────
#  §5  DIAGNOSTICS  (diagnose / status)
# ────────────────────────────────────────────────────────────────────────

cmd_diagnose() {
    local slice container store whole espid mp key

    slice="$(get_root_slice)"
    container="$(get_disk_prefix "$slice")"
    store="$(get_physical_store "$container")"
    whole="$(get_disk_prefix "$store")"
    [[ -z "$whole" ]] && whole="$container"
    espid="$(get_efi_partition "$whole")"

    banner
    echo "root slice       : ${slice:-NOT FOUND}"
    echo "apfs store       : ${store:-none (HFS boot)}"
    echo "host whole disk  : ${whole:-NOT FOUND}"
    echo "esp identifier   : ${espid:-NOT FOUND}"

    if [[ -z "$espid" && -n "$whole" ]]; then
        echo "esp fallback     : multi-disk HFS+ scan…"
        if discover_clover_volume "$whole"; then
            echo "clover volume    : ${ESPID} (HFS+)"
        else
            echo "clover volume    : NOT FOUND"
        fi
    fi

    if [[ -n "$espid" || -n "$ESPID" ]]; then
        [[ -n "$espid" ]] && ESPID="$espid"
        if mp="$(mounted_mp_of_espid)"; then
            echo "current mount    : $mp"
        else
            echo "current mount    : none (idle)"
        fi
    fi

    echo "poll interval    : ${POLL_INTERVAL}s (verify every $((VERIFY_EVERY * POLL_INTERVAL))s)"
    echo "last baseline    : $(grep -c '<key>' "$LAST_XML" 2>/dev/null || echo none)"
    echo "sticky anchor    : $(grep -c '<key>' "$STICKY_XML" 2>/dev/null || echo none)"

    for key in "${HEAL_KEYS[@]}"; do
        if grep -q "<key>${key}</key>" "$STICKY_XML" 2>/dev/null; then
            echo "anchor has       : ${key}  [OK]"
        else
            echo "anchor missing   : ${key}  [--]"
        fi
    done

    # current sticky boot-args (decoded), if any
    if [[ -f "$STICKY_XML" ]]; then
        local bavalue
        bavalue=$(read_bootargs_text "$STICKY_XML")
        if [[ -n "$bavalue" && "$bavalue" != ERR:* ]]; then
            echo "sticky boot-args : ${bavalue}  [data/binary form]"
        elif [[ "$bavalue" == ERR:* ]]; then
            echo "sticky boot-args : (read error: ${bavalue})"
        else
            echo "sticky boot-args : (not set in anchor)"
        fi
    fi

    echo "log dir          : $LOG_DIR"
}

cmd_status() {
    banner

    if launchctl print "system/${DAEMON_LABEL}" >/dev/null 2>&1; then
        ok "Daemon RUNNING"
    elif launchctl list 2>/dev/null | grep -q "$DAEMON_LABEL"; then
        ok "Daemon RUNNING (legacy launchctl)"
    elif [[ -f "$DAEMON_PLIST" ]]; then
        warn "Daemon installed but NOT loaded"
    else
        info "Daemon: not installed"
    fi

    ESPID="$(cat "$ESP_CACHE" 2>/dev/null || true)"
    if [[ -n "$ESPID" ]]; then
        if MP="$(mounted_mp_of_espid)"; then
            info "Clover volume ${ESPID} mounted at: ${MP}"
        else
            info "Clover volume ${ESPID} unmounted (normal idle state)"
        fi
    fi

    [[ -f "$LAST_XML"   ]] && stat -f "baseline      : %N (%z bytes, %Sm)" "$LAST_XML"
    [[ -f "$STICKY_XML" ]] && stat -f "sticky anchor : %N (%z bytes, %Sm)" "$STICKY_XML"
    [[ -f "$LOG_FILE"   ]] && stat -f "log           : %N (%z bytes)" "$LOG_FILE"
}


# ────────────────────────────────────────────────────────────────────────
#  §6  BOOT-ARGS DATA CODEC  (Foundation native - macOS does the work)
#      The JXA helper is the shell equivalent of Swift's
#          let data = Data(someString.utf8)
#      Every encode/decode is performed by Foundation natively.
# ────────────────────────────────────────────────────────────────────────

BA_JS="$LOG_DIR/.clh-ba.js"

write_ba_js() {
    cat > "$BA_JS" <<'JSEOF'
ObjC.import('Foundation');
// argv: [plistPath, key, mode, text?]
//   mode "write": store text as NSData (UTF-8) under key
//   mode "read" : return the text decoded from the data under key
//   mode "b64"  : return the base64 of the data under key
function run(argv) {
    var path = argv[0], key = argv[1], mode = argv[2];

    if (mode === 'write') {
        var text = argv[3];
        var d = $.NSMutableDictionary.dictionaryWithContentsOfFile(path);
        if (d.isNil()) return 'ERR:read';
        var ns = $.NSString.alloc.initWithUTF8String(text);
        var data = ns.dataUsingEncoding($.NSUTF8StringEncoding);
        if (data.isNil()) return 'ERR:encode';
        d.setObjectForKey(data, key);
        if (!d.writeToFileAtomically(path, true)) return 'ERR:write';
        return 'OK';
    }

    var d2 = $.NSDictionary.dictionaryWithContentsOfFile(path);
    if (d2.isNil()) return 'ERR:read';
    var data = d2.objectForKey(key);
    if (data === null || data.isNil()) return 'NONE';

    if (mode === 'read') {
        var s = $.NSString.alloc.initWithDataEncoding(data, $.NSUTF8StringEncoding);
        var out = ObjC.unwrap(s);
        return (out === null || out === undefined) ? '' : String(out);
    }

    if (mode === 'b64') {
        var b64 = data.base64EncodedStringWithOptions(0);
        return String(ObjC.unwrap(b64));
    }

    return 'ERR:mode';
}
JSEOF
}

# write_bootargs_data <text>
#   Stores the text as native <data> (UTF-8 bytes) in the anchor.
#   Foundation handles the encoding; round-trip verified before return.
write_bootargs_data() {
    local new_args="$1"
    write_ba_js

    local msg
    msg="$(osascript -l JavaScript "$BA_JS" "$STICKY_XML" boot-args write "$new_args" 2>/dev/null)"
    if [[ "$msg" != "OK" ]]; then
        warn "boot-args write failed: ${msg:-no output}"
        return 1
    fi

    # keep the anchor in XML form (safe for every reader, Clover included)
    plutil -convert xml1 "$STICKY_XML" >/dev/null 2>&1

    # round-trip verification - the gatekeeper
    local check
    check=$(read_bootargs_text "$STICKY_XML")
    if [[ "$check" == "$new_args" ]]; then
        dbg "boot-args round-trip verified"
        return 0
    fi
    warn "boot-args round-trip MISMATCH: stored='${check}'"
    return 1
}

# read_bootargs_text [plist] -> decoded text of boot-args ("" if absent)
read_bootargs_text() {
    write_ba_js
    osascript -l JavaScript "$BA_JS" "${1:-$STICKY_XML}" boot-args read 2>/dev/null
}

# read_bootargs_b64 [plist] -> base64 of the boot-args data bytes
read_bootargs_b64() {
    write_ba_js
    osascript -l JavaScript "$BA_JS" "${1:-$STICKY_XML}" boot-args b64 2>/dev/null
}

hex_to_percent() {
    printf '%s' "$1" | LC_ALL=C sed -E 's/(..)/%\1/g'
}


# ────────────────────────────────────────────────────────────────────────
#  §6b  LIVE-NVRAM HEALING
#      Restores heal keys MISSING from live NVRAM, reading the anchor.
#      Note: on modern macOS, runtime writes to boot-governed keys are
#      refused - expected; persistence travels through nvram.plist.
# ────────────────────────────────────────────────────────────────────────

heal_live_vars() {
    local quiet="${1:-0}"
    local source_file key tag value hex percent

    (( ${#HEAL_KEYS[@]} )) || return 0

    source_file="$STICKY_XML"
    [[ -f "$source_file" ]] || source_file="$LAST_XML"
    [[ -f "$source_file" ]] || return 0

    local restored=0 refused=0

    for key in "${HEAL_KEYS[@]}"; do
        # Present in live NVRAM -> leave untouched (fresh user choice wins).
        if nvram "$key" >/dev/null 2>&1; then
            continue
        fi

        # v1.5: boot-args go through the Foundation codec
        if [[ "$key" == "boot-args" ]]; then
            local b64
            b64=$(read_bootargs_b64 "$source_file")
            if [[ -z "$b64" || "$b64" == "NONE" || "$b64" == ERR:* ]]; then
                continue
            fi
            hex=$(printf '%s' "$b64" \
                  | openssl base64 -d -A 2>/dev/null \
                  | LC_ALL=C od -An -tx1 | LC_ALL=C tr -d ' \n')
            [[ -z "$hex" ]] && continue
            percent=$(hex_to_percent "$hex")
            if nvram "${key}=${percent}" 2>/dev/null; then
                restored=1
                log "heal: REINJECTED ${key} (data/binary)"
            else
                refused=1
            fi
            continue
        fi

        tag=$(grep -A1 "<key>${key}</key>" "$source_file" 2>/dev/null \
              | sed -n '2p' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

        case "$tag" in
            "<data>"*)
                value=$(awk -v key="$key" '
                    index($0, "<key>" key "</key>") > 0 { grab=1; next }
                    grab==1 && /<data>/                 { grab=2; next }
                    grab==2 && /<\/data>/ {
                        sub(/<\/data>.*/, "")
                        gsub(/[ \t\r]/, "")
                        printf "%s", $0
                        exit
                    }
                    grab==2 { gsub(/[ \t\r]/, ""); printf "%s", $0 }
                ' "$source_file" 2>/dev/null)
                [[ -z "$value" ]] && continue
                hex=$(printf '%s' "$value" \
                      | openssl base64 -d -A 2>/dev/null \
                      | LC_ALL=C od -An -tx1 | LC_ALL=C tr -d ' \n')
                [[ -z "$hex" ]] && continue
                percent=$(hex_to_percent "$hex")
                if nvram "${key}=${percent}" 2>/dev/null; then
                    restored=1
                    log "heal: REINJECTED ${key} (data/binary)"
                else
                    refused=1
                fi
                ;;
            "<string>"*)
                value=$(/usr/libexec/PlistBuddy -c "Print :$key" "$source_file" 2>/dev/null)
                [[ -z "$value" ]] && continue
                if nvram "${key}=${value}" 2>/dev/null; then
                    restored=1
                    log "heal: REINJECTED ${key} (string)"
                else
                    refused=1
                fi
                ;;
        esac
    done

    if (( restored )); then
        (( quiet )) || ok "Live NVRAM healed"
        log "heal applied"
    elif (( refused )); then
        log "heal: live-writes refused by macOS (expected) - file covers it"
    fi
    return 0
}


# ────────────────────────────────────────────────────────────────────────
#  §7  CORE SYNC  (sync_once)
#      dump live NVRAM -> live-wins merge with anchor -> write Clover file
#
#      usage: sync_once [quiet] [merge|prune] [force]
# ────────────────────────────────────────────────────────────────────────

# v1.6.2 - mount_esp(): 4-method cascade for ALL macOS versions.
#   1. diskutil mount -mountOptions nobrowse -mountPoint  (10.11+)
#   2. diskutil mount -mountPoint                        (10.9+)
#   3. legacy mount_msdos / mount_hfs into hidden folder (10.0+, Snow/Lion/ML)
#   4. plain diskutil mount -> /Volumes/<Name>           (visible fallback)
mount_esp() {
    OWN_MOUNT=0

    if MP="$(mounted_mp_of_espid)"; then           # reuse an existing mount
        dbg "mount: reusing $MP"
        return 0
    fi

    mkdir -p "$HIDDEN_MP" 2>/dev/null

    # Methods 1-2: modern diskutil (10.11+ / 10.9+) with explicit mount point
    if diskutil mount -mountOptions nobrowse -mountPoint "$HIDDEN_MP" "$ESPID" >/dev/null 2>&1 \
       || diskutil mount -mountPoint "$HIDDEN_MP" "$ESPID" >/dev/null 2>&1; then
        MP="$HIDDEN_MP"
        OWN_MOUNT=1
        dbg "mount: transient hidden mount at $MP (diskutil modern)"
        return 0
    fi

    # Method 3: legacy mount directly into the hidden folder.
    # Works since 10.0: detect the filesystem, call the right mounter.
    local fstype
    fstype=$(diskutil info -plist "$ESPID" 2>/dev/null \
             | awk -F'<string>' '/FilesystemName|FilesystemType/ {gsub(/<\/string>.*/,"",$2); print $2; exit}')
    dbg "mount: legacy path, filesystem detected = ${fstype:-unknown}"

    local okm=0
    case "$fstype" in
        *MS-DOS*|*FAT*|*msdos*)
            /sbin/mount_msdos /dev/${ESPID} "$HIDDEN_MP" 2>/dev/null && okm=1 ;;
        *HFS*|*Journaled*)
            /sbin/mount_hfs /dev/${ESPID} "$HIDDEN_MP" 2>/dev/null && okm=1 ;;
        *)
            /sbin/mount_msdos /dev/${ESPID} "$HIDDEN_MP" 2>/dev/null && okm=1 \
                || /sbin/mount_hfs /dev/${ESPID} "$HIDDEN_MP" 2>/dev/null && okm=1 ;;
    esac

    if (( okm )); then
        MP="$HIDDEN_MP"
        OWN_MOUNT=1
        dbg "mount: legacy mount at $MP (${fstype:-auto})"
        return 0
    fi

    # Method 4: last resort on old macOS - plain diskutil mount.
    # The volume lands in /Volumes/<Name> (visible, but at least it works).
    if diskutil mount "$ESPID" >/dev/null 2>&1; then
        MP="$(mounted_mp_of_espid)"
        OWN_MOUNT=1
        warn "mount: legacy fallback - mounted at $MP (visible, old macOS)"
        return 0
    fi

    warn "mount: cannot mount ${ESPID} this cycle"
    return 1
}

# v1.6.2 - release_esp(): unmount by DISK IDENTIFIER first (reliable on
# legacy systems where mount points may be odd), then by mount point.
release_esp() {
    if (( OWN_MOUNT )); then
        diskutil unmount force /dev/${ESPID} >/dev/null 2>&1 \
            || diskutil unmount "$MP"      >/dev/null 2>&1 \
            || diskutil unmount force "$MP" >/dev/null 2>&1
        dbg "mount: transient mount released"
    fi
}

# v1.5.1 - ANCHOR RECOVERY
#   If the local anchor is missing (fresh install, uninstall, clean)
#   but the Clover volume still holds a nvram.plist containing heal
#   keys, restore the anchor from it. This recovers boot-device keys
#   AND boot-args from previous runs without any manual step.
seed_anchor_from_esp() {
    [[ -f "$STICKY_XML" ]] && return 0

    resolve_boot_esp_id || return 1

    local mp="" own=0 target
    mp="$(mounted_mp_of_espid || true)"

    if [[ -z "$mp" ]]; then
        mkdir -p "$HIDDEN_MP" 2>/dev/null
        if ! diskutil mount -mountOptions nobrowse -mountPoint "$HIDDEN_MP" "$ESPID" >/dev/null 2>&1 \
           && ! diskutil mount -mountPoint "$HIDDEN_MP" "$ESPID" >/dev/null 2>&1 \
           && ! diskutil mount "$ESPID" >/dev/null 2>&1; then
            dbg "seed: cannot mount ${ESPID} for anchor recovery"
            return 1
        fi
        mp="$(mounted_mp_of_espid || true)"
        own=1
    fi

    target="${mp}/nvram.plist"
    local recovered=0

    if [[ -f "$target" ]]; then
        local key
        for key in "${HEAL_KEYS[@]}"; do
            if grep -q "<key>${key}</key>" "$target" 2>/dev/null; then
                cp "$target" "$STICKY_XML" 2>/dev/null
                recovered=1
                log "seed: anchor RECOVERED from existing ESP nvram.plist"
                break
            fi
        done
    fi

    if (( own )); then
        diskutil unmount force /dev/${ESPID} >/dev/null 2>&1 \
            || diskutil unmount "$mp" >/dev/null 2>&1 \
            || diskutil unmount force "$mp" >/dev/null 2>&1
    fi

    if (( recovered )); then
        ok "Anchor recovered from existing Clover file (boot keys preserved)"
    else
        dbg "seed: no valid anchor source on ESP"
    fi
    return 0
}

# v1.5 - migrate legacy <string> boot-args to native <data> (once)
migrate_bootargs_to_data() {
    [[ -f "$STICKY_XML" ]] || return 0

    local tag
    tag=$(grep -A1 '<key>boot-args</key>' "$STICKY_XML" 2>/dev/null \
          | sed -n '2p' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
    [[ "$tag" == "<string>"* ]] || return 0          # already data / absent

    local text
    text=$(/usr/libexec/PlistBuddy -c "Print :boot-args" "$STICKY_XML" 2>/dev/null)
    [[ -z "$text" ]] && return 0

    if write_bootargs_data "$text"; then
        log "migrate: boot-args converted string -> data (binary form)"
        ok "boot-args migrated to binary <data> form"
    fi
}

sync_once() {
    local quiet="${1:-0}"
    local mode="${2:-merge}"
    local force="${3:-0}"

    resolve_boot_esp_id || return 1

    # v1.5.1: recover the anchor from the ESP if it is missing
    if [[ ! -f "$STICKY_XML" ]]; then
        seed_anchor_from_esp
    fi

    # v1.5.2: one-time migration of legacy string boot-args
    migrate_bootargs_to_data

    local tmp="$LOG_DIR/nvram.new.xml"
    if ! nvram -x -p > "$tmp" 2>/dev/null; then
        rm -f "$tmp"; return 1
    fi
    [[ -s "$tmp" ]]          || { rm -f "$tmp"; return 1; }
    grep -q '<dict>' "$tmp"  || { rm -f "$tmp"; return 1; }
    plutil -lint "$tmp" >/dev/null 2>&1 \
                             || { rm -f "$tmp"; return 1; }

    local total
    total=$(grep -c '<key>' "$tmp" 2>/dev/null)
    (( total > 0 ))          || { rm -f "$tmp"; return 1; }

    if [[ "$force" != "1" && -f "$LAST_XML" ]] && cmp -s "$tmp" "$LAST_XML"; then
        if [[ ! -f "$STICKY_XML" ]]; then
            # v1.5.1: never report "all good" while the anchor is missing
            dbg "no-change (${total} vars) BUT anchor missing - forcing seed"
            force=1
        else
            (( quiet )) || info "NVRAM unchanged (${total} vars) - nothing touched."
            dbg "no-change (${total} vars)"
            rm -f "$tmp"
            return 0
        fi
    fi

    local final="$LOG_DIR/nvram.final.xml"
    local anchor="$STICKY_XML"
    [[ -f "$anchor" ]] || anchor="$LAST_XML"

    if [[ "$mode" == "prune" ]]; then
        cp "$tmp" "$final"
    elif [[ ! -f "$anchor" ]]; then
        cp "$tmp" "$final"                        # very first run
    else
        cp "$anchor" "$final"

        local live_key ex skip
        while IFS= read -r live_key; do
            [[ -n "$live_key" ]] || continue
            skip=0
            for ex in "${MERGE_EXCLUDE[@]}"; do
                if [[ "$live_key" == "$ex" ]]; then
                    skip=1
                    break
                fi
            done
            (( skip )) && continue
            /usr/libexec/PlistBuddy -c "Delete :${live_key}" "$final" >/dev/null 2>&1 || true
        done < <(grep -o '<key>[^<]*</key>' "$tmp" | sed 's/<[^>]*>//g')

        # stdout+stderr both captured: silences the harmless
        # "Duplicate Entry Was Skipped" notices (colon keys like
        # prev-lang:kbd cannot be Deleted via PlistBuddy paths)
        local merge_out
        if ! merge_out=$(/usr/libexec/PlistBuddy -c "Merge $tmp" "$final" 2>&1); then
            warn "PlistBuddy merge failed - previous Clover file kept, will retry."
            log "merge-fail keep-prev (${merge_out})"
            rm -f "$tmp" "$final"
            return 0
        fi
    fi

    mount_esp || { rm -f "$tmp" "$final"; return 1; }

    # Sanity: the mounted volume should host Clover.
    if [[ ! -d "$MP/EFI/CLOVER" ]]; then
        warn "Mounted volume ${ESPID} has no EFI/CLOVER - multi-disk search…"
        log "clover-search: ${ESPID} lacks EFI/CLOVER"
        release_esp
        if discover_clover_volume "$HOST_DISK"; then
            (( quiet )) || ok "Clover volume located: ${ESPID} (HFS+)"
            mount_esp || { rm -f "$tmp" "$final"; return 1; }
        fi
        if [[ ! -d "$MP/EFI/CLOVER" ]]; then
            # Last resort: some builds read nvram.plist from the volume
            # root even without a Clover folder there.
            warn "No EFI/CLOVER found - writing volume root anyway."
            log "clover-search: fallback - writing volume root"
        fi
    fi

    if ! install -m 644 "$final" "${MP}/nvram.plist" 2>/dev/null; then
        warn "write failed on ${MP}/nvram.plist"
        rm -f "$tmp" "$final"
        release_esp
        return 1
    fi
    sync

    # ---- update local state ----------------------------------------------
    cp "$tmp" "$LAST_XML" 2>/dev/null

    local key has_heal=0
    for key in "${HEAL_KEYS[@]}"; do
        if grep -q "<key>${key}</key>" "$final" 2>/dev/null; then
            has_heal=1
            break
        fi
    done
    if (( has_heal )); then
        cp "$final" "$STICKY_XML" 2>/dev/null     # refresh anchor with NEW values
        log "sticky anchor updated"
    fi

    # ---- done -------------------------------------------------------------
    local nkeys
    nkeys=$(grep -c '<key>' "$final")
    (( quiet )) || ok "Persisted ${nkeys} variable(s) -> ${MP}/nvram.plist"
    log "sync ${mode} force=${force} (${nkeys} vars) -> ${ESPID}"

    rm -f "$tmp" "$final"
    release_esp
    return 0
}


# ────────────────────────────────────────────────────────────────────────
#  §8  CLOVER FILE VERIFICATION
#      If the on-volume file lost anchor keys (external amputation),
#      force a rebuild. Runs every VERIFY_EVERY cycles.
# ────────────────────────────────────────────────────────────────────────

verify_esp_file() {
    [[ -f "$STICKY_XML" ]] || return 0

    resolve_boot_esp_id || return 1

    local key anchor_has_key=0
    for key in "${HEAL_KEYS[@]}"; do
        if grep -q "<key>${key}</key>" "$STICKY_XML" 2>/dev/null; then
            anchor_has_key=1
            break
        fi
    done
    (( anchor_has_key )) || return 0

    MP="$(mounted_mp_of_espid || true)"
    OWN_MOUNT=0

    if [[ -z "$MP" ]]; then
        mount_esp || return 1
    fi

    local need_rebuild=0
    local target="${MP}/nvram.plist"

    if [[ ! -f "$target" ]]; then
        need_rebuild=1
    else
        for key in "${HEAL_KEYS[@]}"; do
            if grep -q "<key>${key}</key>" "$STICKY_XML" \
               && ! grep -q "<key>${key}</key>" "$target"; then
                need_rebuild=1
                break
            fi
        done
    fi

    release_esp

    if (( need_rebuild )); then
        log "verify: Clover file lacks anchor keys - forced rebuild"
        sync_once 1 merge 1
    else
        dbg "verify: Clover file healthy"
    fi
}


# ────────────────────────────────────────────────────────────────────────
#  §9  HOUSEKEEPING
# ────────────────────────────────────────────────────────────────────────

rotate_log() {
    [[ -f "$LOG_FILE" ]] || return 0
    local size
    size=$(stat -f%z "$LOG_FILE" 2>/dev/null || echo 0)
    if (( size >= 409600 )); then              # 400 KB cap
        tail -c 150000 "$LOG_FILE" > "${LOG_FILE}.tmp" 2>/dev/null \
            && mv "${LOG_FILE}.tmp" "$LOG_FILE"
        log "log rotated (${size} -> $(stat -f%z "$LOG_FILE" 2>/dev/null) bytes)"
    fi
}

cleanup_temp_files() {
    rm -f "$LOG_DIR/nvram.new.xml"   2>/dev/null
    rm -f "$LOG_DIR/nvram.final.xml" 2>/dev/null
    rm -f "$LOG_DIR/nvram.fast.xml"  2>/dev/null
}


# ────────────────────────────────────────────────────────────────────────
#  §10  DAEMON
# ────────────────────────────────────────────────────────────────────────

on_shutdown() {                                # SIGTERM / SIGINT handler
    log "shutdown signal received - final flush…"
    if ! sync_once 1 merge >/dev/null 2>&1; then
        sleep 1
        sync_once 1 merge >/dev/null 2>&1      # one retry during teardown
    fi
    heal_live_vars 1 >/dev/null 2>&1
    log "final flush done"
    exit 0
}

cmd_watch() {
    banner
    info "Watch: check every ${POLL_INTERVAL}s · live-wins merge · self-heal · flush-on-shutdown."

    trap on_shutdown TERM INT

    cleanup_temp_files

    # Startup self-heal pass: repair any external amputation after boot.
    sleep 5
    sync_once 1 merge 1 >/dev/null 2>&1

    local fast="$LOG_DIR/nvram.fast.xml"
    local cycle=0

    while true; do
        cycle=$((cycle + 1))

        heal_live_vars 1 >/dev/null 2>&1

        if nvram -x -p > "$fast" 2>/dev/null; then
            if ! cmp -s "$fast" "$LAST_XML"; then
                sync_once 1 merge >/dev/null 2>&1 || log "hiccup"
            fi
        fi
        rm -f "$fast" 2>/dev/null

        if (( cycle % VERIFY_EVERY == 0 )); then
            verify_esp_file >/dev/null 2>&1 || log "verify hiccup"
            rotate_log
        fi

        sleep "$POLL_INTERVAL"
    done
}

cmd_install() {
    banner
    mkdir -p /usr/local/bin
    install -m 755 "$SELF_PATH" "$INSTALL_BIN"

    # migrate state from the old CloverEFI folder if present
    if [[ -d "/Library/Logs/CloverEFI" && ! -d "$LOG_DIR" ]]; then
        mv "/Library/Logs/CloverEFI" "$LOG_DIR"
        info "Migrated state folder: /Library/Logs/CloverEFI -> ${LOG_DIR}"
    fi

    cat > "$DAEMON_PLIST" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.clover.nvramhook.daemon</string>

    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/clover-logout-hook</string>
        <string>watch</string>
    </array>

    <key>EnvironmentVariables</key>
    <dict>
        <key>CLOVER_POLL_INTERVAL</key>
        <string>${POLL_INTERVAL}</string>
    </dict>

    <key>RunAtLoad</key>
    <true/>

    <key>KeepAlive</key>
    <true/>

    <key>StandardOutPath</key>
    <string>${LOG_DIR}/daemon.out</string>

    <key>StandardErrorPath</key>
    <string>${LOG_DIR}/daemon.err</string>
</dict>
</plist>
PLIST

    chmod 644 "$DAEMON_PLIST"

    launchctl bootout   "system/${DAEMON_LABEL}" 2>/dev/null
    launchctl bootstrap system "$DAEMON_PLIST" 2>/dev/null \
        || launchctl load -w "$DAEMON_PLIST"

    ok "Installed v${VERSION} -> ${INSTALL_BIN}"

    # v1.5.1 - GUARANTEED SEED: recover anchor from ESP if needed,
    # then force one full sync so anchor + file exist immediately.
    info "Seeding anchor and Clover file…"
    sync_once 1 merge 1

    ok "Ready. Logs: ${LOG_DIR}"
}

cmd_uninstall() {
    # stop the daemon (modern + legacy launchctl paths)
    launchctl bootout   "system/${DAEMON_LABEL}" 2>/dev/null
    launchctl unload    "$DAEMON_PLIST" 2>/dev/null
    launchctl remove    "$DAEMON_LABEL" 2>/dev/null
    rm -f "$DAEMON_PLIST"

    # release our transient mount, if any
    if is_mounted_at "$HIDDEN_MP"; then
        diskutil unmount force "$HIDDEN_MP" >/dev/null 2>&1
    fi
    rmdir "$HIDDEN_MP" 2>/dev/null

    # remove the installed binary
    if [[ -f "$INSTALL_BIN" ]]; then
        rm -f "$INSTALL_BIN"
        ok "Removed: ${INSTALL_BIN}"
    fi

    # remove ONLY the CloverHook folder (never Clover's own dirs)
    case "$LOG_DIR" in
        /Library/Logs/CloverHook|/tmp/clover-nvramhook)
            if [[ -d "$LOG_DIR" ]]; then
                rm -rf "$LOG_DIR"
                ok "Removed: ${LOG_DIR}"
            fi
            ;;
    esac

    # v1.5.3: also purge any stale /tmp fallback state
    rm -rf "/tmp/clover-nvramhook" 2>/dev/null

    ok "CloverLogoutHook fully uninstalled."

    info "Note: the Clover volume's nvram.plist was left in place (harmless)."
    info "Your master copy remains in ~/CloverLogoutHook/ (delete it if unwanted)."
}


# ────────────────────────────────────────────────────────────────────────
#  §11  MAINTENANCE
# ────────────────────────────────────────────────────────────────────────

cmd_clean() {
    need_root clean

    cleanup_temp_files
    rm -f "$LAST_XML"    2>/dev/null
    rm -f "$STICKY_XML"  2>/dev/null
    : > "$LOG_FILE"                2>/dev/null || true
    [[ -f "${LOG_DIR}/daemon.out" ]] && : > "${LOG_DIR}/daemon.out"
    [[ -f "${LOG_DIR}/daemon.err" ]] && : > "${LOG_DIR}/daemon.err"

    ok "Cleaned: logs truncated, tmp files, snapshots and sticky anchor removed."
    warn "Anchor is RESET - make ONE Startup Disk click to re-seed it."
}

cmd_prune() {
    need_root prune
    sync_once 0 prune || die "prune failed"
}

# v1.5 - set boot-args from the terminal (anchor becomes source of truth).
# Clover reads boot-args from NVRAM after importing nvram.plist, so the
# value stored here wins at next boot - without touching config.plist.
cmd_setargs() {
    need_root setargs
    [[ $# -ge 1 ]] || die "usage: sudo $0 setargs \"-v keepsyms=1 ...\""
    local new_args="$*"

    if [[ ! -f "$STICKY_XML" ]]; then
        sync_once 1 merge 1
    fi
    if [[ ! -f "$STICKY_XML" ]]; then
        # v1.6.1: fresh rig with no Startup Disk click yet -> create
        # a minimal anchor; setargs itself provides the first heal key.
        printf '<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n<plist version="1.0">\n<dict>\n</dict>\n</plist>\n' > "$STICKY_XML" 2>/dev/null \
            || die "cannot create anchor (permissions?)"
        log "seed: minimal anchor created by setargs (fresh rig)"
    fi

    # v1.6: write as native <data> via the Foundation codec
    if ! write_bootargs_data "$new_args"; then
        die "boot-args write failed - anchor NOT modified inconsistently"
    fi

    # mirror into live NVRAM as BINARY form (allowed on old macOS)
    local b64 hex percent
    b64=$(read_bootargs_b64 "$STICKY_XML")
    hex=$(printf '%s' "$b64" \
          | openssl base64 -d -A 2>/dev/null \
          | LC_ALL=C od -An -tx1 | LC_ALL=C tr -d ' \n')
    percent=$(hex_to_percent "$hex")
    if nvram "boot-args=${percent}" 2>/dev/null; then
        info "Live NVRAM updated too, in binary form (this OS allows it)."
    else
        info "Live NVRAM refused (expected on modern macOS) - file wins at next boot."
    fi

    # force-write the anchor (with new args) to the Clover volume
    sync_once 1 merge 1

    ok "boot-args set as <data> (binary form), round-trip verified:"
    echo "      ${new_args}"
    info "Reboot to apply."
}


# ────────────────────────────────────────────────────────────────────────
#  §12  COMMAND DISPATCHER
# ────────────────────────────────────────────────────────────────────────

setup_log_dir

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)/$(basename "${BASH_SOURCE[0]:-$0}")"

CMD="${1:-dump}"
(( $# )) && shift

case "$CMD" in

    # --- information (no sudo needed) ------------------------------------
    version)   banner ;;
    diagnose)  cmd_diagnose ;;
    status)    cmd_status ;;

    # --- maintenance (sudo) ----------------------------------------------
    clean)     cmd_clean ;;
    prune)     cmd_prune ;;
    setargs)   cmd_setargs "$@" ;;

    # --- persistence (sudo) ----------------------------------------------
    dump|"")   need_root dump; sync_once 0 merge; heal_live_vars 0 ;;
    watch)     need_root watch; cmd_watch ;;
    install)   need_root install; cmd_install ;;
    uninstall) need_root uninstall; cmd_uninstall ;;

    # --- unknown ----------------------------------------------------------
    *)         banner
               echo "Usage: sudo $0 [dump | setargs \"-v ...\" | watch | install | uninstall | clean | prune]"
               echo "       $0 [status | diagnose | version]" ;;
esac
