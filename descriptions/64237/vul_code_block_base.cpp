char arrBuff[4] = {static_cast<char>((refID >> 24) & 0xFF), static_cast<char>((refID >> 16) & 0xFF),
                                       static_cast<char>((refID >> 8) & 0xFF), static_cast<char>((refID) & 0xFF)};
                    return arrBuff;