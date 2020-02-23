#ifdef DEBUG_CLOVER
DBG("g_str = %s\n", g_str.data());
DBG("g_str2 = %s\n", g_str2.data());
extern XStringW global_str1;
DBG("global_str1 = %s\n", global_str1.data());
extern XStringW global_str2;
DBG("global_str2 = %s\n", global_str2.data());
{
//	XStringW str(L"local str value");
//	DBG("str = %s\n", str.data());
//	str.StrCat(L" appended text");
//	DBG("str = %s, len=%d\n", str.data(), str.length());
//
//	XStringW str2(str);
//	DBG("str2 = %s\n", str2.data());
//	str2.StrnCpy(str.data(), 2);
//	DBG("str2 = %s\n", str2.data());
//	str2.StrnCat(L"2ndtext", 2);
//	DBG("str2 = %s\n", str2.data());
//	str2.Insert(1, str);
//	DBG("str2 = %s\n", str2.data());
//	str2 += L"3rdtext";
//	DBG("str2 = %s\n", str2.data());
//
//	XStringW* str3 = new XStringW();
//	*str3 = L"str3data";
//	DBG("str3 = %s\n", str3->data());
//	delete str3;
}
//
destruct_globals_objects(NULL); // That should be done just before quitting clover module. Now, it's just for test.
DBG("press");
PauseForKey(L"press");
#endif

