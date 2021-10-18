/*
 * PlatformdataModels.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

//#ifndef PLATFORM_PLATFORMDATAMODELS_H_
//#define PLATFORM_PLATFORMDATAMODELS_H_



//https://webdevdesigner.com/q/is-there-a-simple-way-to-convert-c-enum-to-string-30844/
//enum Colours {
//#   define X(a) a,
//#   include "colours.def"
//#   undef X
//    ColoursCount
//};
//
//char const* const MachineModelName[] = {
//#   define X(a) #a,
//#   include "colours.def"
//#   undef X
//    0
//};
//Cependant, je préfère la méthode suivante, de sorte qu'il est possible de modifier la chaîne un peu.
//
//#define X(a, b) a,
//#define X(a, b) b,
//
//X(Red, "red")
//X(Green, "green")
//// etc.



  DEFINE_ENUM(MacBook11, "MacBook1,1")
  DEFINE_ENUM(MacBook21, "MacBook2,1")
  DEFINE_ENUM(MacBook31, "MacBook3,1")
  DEFINE_ENUM(MacBook41, "MacBook4,1")
  DEFINE_ENUM(MacBook51, "MacBook5,1")
  DEFINE_ENUM(MacBook52, "MacBook5,2")
  DEFINE_ENUM(MacBook61, "MacBook6,1")
  DEFINE_ENUM(MacBook71, "MacBook7,1")
  DEFINE_ENUM(MacBook81, "MacBook8,1")
  DEFINE_ENUM(MacBook91, "MacBook9,1")
  DEFINE_ENUM(MacBook101, "MacBook10,1")
  DEFINE_ENUM(MacBookPro11, "MacBookPro1,1")
  DEFINE_ENUM(MacBookPro12, "MacBookPro1,2")
  DEFINE_ENUM(MacBookPro21, "MacBookPro2,1")
  DEFINE_ENUM(MacBookPro22, "MacBookPro2,2")
  DEFINE_ENUM(MacBookPro31, "MacBookPro3,1")
  DEFINE_ENUM(MacBookPro41, "MacBookPro4,1")
  DEFINE_ENUM(MacBookPro51, "MacBookPro5,1")
  DEFINE_ENUM(MacBookPro52, "MacBookPro5,2")
  DEFINE_ENUM(MacBookPro53, "MacBookPro5,3")
  DEFINE_ENUM(MacBookPro54, "MacBookPro5,4")
  DEFINE_ENUM(MacBookPro55, "MacBookPro5,5")
  DEFINE_ENUM(MacBookPro61, "MacBookPro6,1")
  DEFINE_ENUM(MacBookPro62, "MacBookPro6,2")
  DEFINE_ENUM(MacBookPro71, "MacBookPro7,1")
  DEFINE_ENUM(MacBookPro81, "MacBookPro8,1")
  DEFINE_ENUM(MacBookPro82, "MacBookPro8,2")
  DEFINE_ENUM(MacBookPro83, "MacBookPro8,3")
  DEFINE_ENUM(MacBookPro91, "MacBookPro9,1")
  DEFINE_ENUM(MacBookPro92, "MacBookPro9,2")
  DEFINE_ENUM(MacBookPro101, "MacBookPro10,1")
  DEFINE_ENUM(MacBookPro102, "MacBookPro10,2")
  DEFINE_ENUM(MacBookPro111, "MacBookPro11,1")
  DEFINE_ENUM(MacBookPro112, "MacBookPro11,2")
  DEFINE_ENUM(MacBookPro113, "MacBookPro11,3")
  DEFINE_ENUM(MacBookPro114, "MacBookPro11,4")
  DEFINE_ENUM(MacBookPro115, "MacBookPro11,5")
  DEFINE_ENUM(MacBookPro121, "MacBookPro12,1")
  DEFINE_ENUM(MacBookPro131, "MacBookPro13,1")
  DEFINE_ENUM(MacBookPro132, "MacBookPro13,2")
  DEFINE_ENUM(MacBookPro133, "MacBookPro13,3")
  DEFINE_ENUM(MacBookPro141, "MacBookPro14,1")
  DEFINE_ENUM(MacBookPro142, "MacBookPro14,2")
  DEFINE_ENUM(MacBookPro143, "MacBookPro14,3")
  DEFINE_ENUM(MacBookPro151, "MacBookPro15,1")
  DEFINE_ENUM(MacBookPro152, "MacBookPro15,2")
  DEFINE_ENUM(MacBookPro153, "MacBookPro15,3")
  DEFINE_ENUM(MacBookPro154, "MacBookPro15,4")
  DEFINE_ENUM(MacBookPro161, "MacBookPro16,1")
  DEFINE_ENUM(MacBookPro162, "MacBookPro16,2")
  DEFINE_ENUM(MacBookPro163, "MacBookPro16,3")
  DEFINE_ENUM(MacBookPro164, "MacBookPro16,4")
  DEFINE_ENUM(MacBookAir11, "MacBookAir1,1")
  DEFINE_ENUM(MacBookAir21, "MacBookAir2,1")
  DEFINE_ENUM(MacBookAir31, "MacBookAir3,1")
  DEFINE_ENUM(MacBookAir32, "MacBookAir3,2")
  DEFINE_ENUM(MacBookAir41, "MacBookAir4,1")
  DEFINE_ENUM(MacBookAir42, "MacBookAir4,2")
  DEFINE_ENUM(MacBookAir51, "MacBookAir5,1")
  DEFINE_ENUM(MacBookAir52, "MacBookAir5,2")
  DEFINE_ENUM(MacBookAir61, "MacBookAir6,1")
  DEFINE_ENUM(MacBookAir62, "MacBookAir6,2")
  DEFINE_ENUM(MacBookAir71, "MacBookAir7,1")
  DEFINE_ENUM(MacBookAir72, "MacBookAir7,2")
  DEFINE_ENUM(MacBookAir81, "MacBookAir8,1")
  DEFINE_ENUM(MacBookAir82, "MacBookAir8,2")
  DEFINE_ENUM(MacBookAir91, "MacBookAir9,1")
  DEFINE_ENUM(MacMini11, "Macmini1,1")
  DEFINE_ENUM(MacMini21, "Macmini2,1")
  DEFINE_ENUM(MacMini31, "Macmini3,1")
  DEFINE_ENUM(MacMini41, "Macmini4,1")
  DEFINE_ENUM(MacMini51, "Macmini5,1")
  DEFINE_ENUM(MacMini52, "Macmini5,2")
  DEFINE_ENUM(MacMini53, "Macmini5,3")
  DEFINE_ENUM(MacMini61, "Macmini6,1")
  DEFINE_ENUM(MacMini62, "Macmini6,2")
  DEFINE_ENUM(MacMini71, "Macmini7,1")
  DEFINE_ENUM(MacMini81, "Macmini8,1")
  DEFINE_ENUM(iMac41, "iMac4,1")
  DEFINE_ENUM(iMac42, "iMac4,2")
  DEFINE_ENUM(iMac51, "iMac5,1")
  DEFINE_ENUM(iMac52, "iMac5,2")
  DEFINE_ENUM(iMac61, "iMac6,1")
  DEFINE_ENUM(iMac71, "iMac7,1")
  DEFINE_ENUM(iMac81, "iMac8,1")
  DEFINE_ENUM(iMac91, "iMac9,1")
  DEFINE_ENUM(iMac101, "iMac10,1")
  DEFINE_ENUM(iMac111, "iMac11,1")
  DEFINE_ENUM(iMac112, "iMac11,2")
  DEFINE_ENUM(iMac113, "iMac11,3")
  DEFINE_ENUM(iMac121, "iMac12,1")
  DEFINE_ENUM(iMac122, "iMac12,2")
  DEFINE_ENUM(iMac131, "iMac13,1")
  DEFINE_ENUM(iMac132, "iMac13,2")
  DEFINE_ENUM(iMac133, "iMac13,3")
  DEFINE_ENUM(iMac141, "iMac14,1")
  DEFINE_ENUM(iMac142, "iMac14,2")
  DEFINE_ENUM(iMac143, "iMac14,3")
  DEFINE_ENUM(iMac144, "iMac14,4")
  DEFINE_ENUM(iMac151, "iMac15,1")
  DEFINE_ENUM(iMac161, "iMac16,1")
  DEFINE_ENUM(iMac162, "iMac16,2")
  DEFINE_ENUM(iMac171, "iMac17,1")
  DEFINE_ENUM(iMac181, "iMac18,1")
  DEFINE_ENUM(iMac182, "iMac18,2")
  DEFINE_ENUM(iMac183, "iMac18,3")
  DEFINE_ENUM(iMac191, "iMac19,1")
  DEFINE_ENUM(iMac192, "iMac19,2")
  DEFINE_ENUM(iMac201, "iMac20,1")
  DEFINE_ENUM(iMac202, "iMac20,2")
  DEFINE_ENUM(iMacPro11, "iMacPro1,1")
  DEFINE_ENUM(MacPro11, "MacPro1,1")
  DEFINE_ENUM(MacPro21, "MacPro2,1")
  DEFINE_ENUM(MacPro31, "MacPro3,1")
  DEFINE_ENUM(MacPro41, "MacPro4,1")
  DEFINE_ENUM(MacPro51, "MacPro5,1")
  DEFINE_ENUM(MacPro61, "MacPro6,1")
  DEFINE_ENUM(MacPro71, "MacPro7,1")
  DEFINE_ENUM(Xserve11, "Xserve1,1")
  DEFINE_ENUM(Xserve21, "Xserve2,1")
  DEFINE_ENUM(Xserve31, "Xserve3,1")

//#endif
