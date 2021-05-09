/*
 * secureboot.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef ENTRY_SCAN_SECUREBOOT_H_
#define ENTRY_SCAN_SECUREBOOT_H_



// Secure boot policies
// Deny all images
#define SECURE_BOOT_POLICY_DENY      (0)
// Allow all images
#define SECURE_BOOT_POLICY_ALLOW     (1)
// Query the user to choose action
#define SECURE_BOOT_POLICY_QUERY     (2)
// Insert signature into db
#define SECURE_BOOT_POLICY_INSERT    (3)
// White list
#define SECURE_BOOT_POLICY_WHITELIST (4)
// Black list
#define SECURE_BOOT_POLICY_BLACKLIST (5)
// User policy, white and black list with query
#define SECURE_BOOT_POLICY_USER      (6)

#ifdef ENABLE_SECURE_BOOT
void InitializeSecureBoot(void);
#endif

#endif /* ENTRY_SCAN_SECUREBOOT_H_ */
