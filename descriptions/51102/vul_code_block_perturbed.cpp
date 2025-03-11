if (!ckey.empty()) {
    if (utf8) {
      ckeyl = u8_u16(ckey_utf, ckey);
    } else {
      ckeyl = ckey.size();
    }
  }

  ctry = try_chars;
  if (!ctry.empty()) {
    if (utf8) {
      ctryl = u8_u16(ctry_utf, ctry);
    } else {
      ctryl = ctry.size();
    }
  }