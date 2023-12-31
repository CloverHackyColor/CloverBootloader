# EDK II Image Loader documentation

**Please note that this is an early work-in-progress. Correctness and completeness are not guaranteed.**

## 1. Image Section Table well-formedness
An Image Section Table is considered well-formed if and only if:
1. The Image Section Table is sorted by Image section RVA in ascending order.
2. All Image sections are disjoint in the Image memory space.
3. For PE/COFF Images, all Image sections are in bounds of the Image memory space defined by the Image headers.
4. All Image sections are in bounds of the raw file.

Additionally, based on PCD policy values, the following constraints may be added:

5. All Image sections are aligned by the Image section alignment.
6. The first Image section is adjacent to the aligned Image headers, or it is the start of the Image memory space.
7. All Image sections are pairwise adjacent (considering aligned size) in the Image memory space.
8. All Image sections are not executable and writable at the same time.

### 1.2. Rationales

|| Origin | Rationale | Known violations |
|---|---|---|---|
|1 | PE | Allows for efficient verification of constraint 2. | None |
|2 | PE | Mitigates accidental Image corruption and ensures the same deterministic Image memory space contents invariant of Image section load order. | None |
|3 | PE | Provides hardening against (otherwise potentially incorrect) assumptions regarding the Image header information. | None |
|4 | PE | Mitigates out-of-bounds accesses. | None |
|5 | PE | Ensures data is aligned at their optimal boundary and provides hardening against (otherwise potentially incorrect) assumptions regarding the Image header information. | Old Apple Mac OS X bootloaders, old iPXE OPROMs |
|6 | EDK II | Allows both the Image Headers to be or to not be loaded. Loading the Image Headers is the default behaviour for all common PE/COFF Image loaders, however it is not required, and the data may be used to more easily identify locations within the Image memory space. | See 5 |
|7 | PE | Provides hardening against unintentional gaps in the Image memory space that may be handled incorrectly. | See 5 |
|8 | EDK II | Provides hardening against accidental and malicious code modifications. The concept is well-known as W^X. | None |

## 2. Image memory permissions
The Image memory space is protected as follows:
1. Image Headers are protected as read-only data.
2. Image sections are protected according to their defined permissions.
3. Any gaps between Image sections are protected as read-only data.
4. The Image trailer is protected as read-only data.

Due to the explicit handling of Image Headers, gaps, and Image trailer, the full Image memory space has well-defined permissions. To embrace the W^X pattern, and to mitigate attacks (e.g. format string exploits), the tightest permissions are chosen for all Image memory permission segments.
