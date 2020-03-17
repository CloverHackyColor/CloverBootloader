
//
// Hibernate.c
//
/** Returns TRUE if given macOS on given volume is hibernated
 *  (/private/var/vm/sleepimage exists and it's modification time is close to volume modification time).
 */
BOOLEAN
IsOsxHibernated (
  IN LOADER_ENTRY    *Entry
  );
