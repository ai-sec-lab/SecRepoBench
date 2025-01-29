if (!ckey.empty()) {
    if (utf8) {
      int len = u8_u16(ckey_utf, ckey);
      if (len != -1) {
        ckeyl = len;
      }
    } else {
      ckeyl = ckey.size();
    }
  }

  ctry = tryme;
  if (!ctry.empty()) {
    if (utf8) {
      int len = u8_u16(ctry_utf, ctry);
      if (len != -1) {
        ctryl = len;
      }
    } else {
      ctryl = ctry.size();
    }
  }