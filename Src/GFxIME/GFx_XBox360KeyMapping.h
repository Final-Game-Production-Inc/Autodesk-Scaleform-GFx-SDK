
XIME_KEYMAPPING GFxIMEManagerXBox360::InputKeymap[] = {

    { MODE_INPUT | MODE_JKT, VK_SHIFT,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT | XINPUT_KEYSTROKE_SHIFT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::SwitchLanguage },  //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_RETURN,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_COMPLETE, (IMECALLBACK)GFxIMEManagerXBox360::Complete },         //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_SEPARATOR,XINPUT_KEYSTROKE_KEYDOWN,0, XIME_KEY_COMPLETE, (IMECALLBACK)GFxIMEManagerXBox360::Complete },         //for Japanese, Hangul, Bopomofo, special conversion
    { MODE_INPUT | MODE_JKT, VK_BACK,    XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_BACKSPACE, NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_DELETE,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_DELETE,    NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_HOME,    XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_HOME,      NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_END,     XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_END,       NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_RIGHT,   XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_RIGHT,     NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_UP,    XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_LEFT,      NULL },  
    { MODE_INPUT | MODE_JKT, VK_DOWN,   XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_RIGHT,     NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_LEFT,    XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_LEFT,      NULL },                               //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_ESCAPE,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_FLUSH, NULL },                                   //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JKT, VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },    
    { MODE_INPUT | MODE_JKT, VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::SwitchLanguage },  //for Japanese, Hangul, Bopomofo
    { MODE_INPUT | MODE_JP,  VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN, XINPUT_KEYSTROKE_SHIFT | XINPUT_KEYSTROKE_CTRL | XINPUT_KEYSTROKE_ALT,   XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },         //for Japanese
    { MODE_INPUT | MODE_JP,  VK_CONVERT, XINPUT_KEYSTROKE_KEYDOWN,  XINPUT_KEYSTROKE_CTRL | XINPUT_KEYSTROKE_ALT,   XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },         //for Japanese, Bopomofo
    { MODE_INPUT | MODE_JP,  VK_CONVERT, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT , XINPUT_KEYSTROKE_CTRL | XINPUT_KEYSTROKE_ALT,   XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },         //for Japanese, Bopomofo
    { MODE_INPUT | MODE_JP,  VK_NONCONVERT, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_KATAKANA_CANDIDATE , (IMECALLBACK)GFxIMEManagerXBox360::NonConvert },            //for Japanese                                                   
    { MODE_INPUT | MODE_JP,  VK_F6,      XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_HIRAGANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },                   //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  VK_F7,      XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_KATAKANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },                   //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  VK_F9,      XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_ALPHANUMERIC_FULL_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },    //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  VK_F10,     XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_ALPHANUMERIC_HALF_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },    //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  L'U',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_HIRAGANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },   //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  L'I',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_KATAKANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },   //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  L'P',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_ALPHANUMERIC_FULL_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },          //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  L'O',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_ALPHANUMERIC_HALF_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },          //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  VK_KANA,    XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_MODE_JP_ROMAJI_HIRAGANA_WITH_FULLWIDTH_ALPHANUMERIC, (IMECALLBACK)GFxIMEManagerXBox360::ChangeInputMode },          //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  VK_KANA,    XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0, XIME_MODE_JP_ROMAJI_KATAKANA_WITH_FULLWIDTH_ALPHANUMERIC, (IMECALLBACK)GFxIMEManagerXBox360::ChangeInputMode },          //for Japanese, special conversion
    { MODE_INPUT | MODE_JP,  VK_KANA,    XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, XIME_MODE_JP_KANAINPUT_WITH_KEYSTROKE_SHIFTFLAG_OFF, (IMECALLBACK)GFxIMEManagerXBox360::ToggleKeyboardLayout },          //for Japanese, special conversion

    { MODE_INPUT | MODE_JP,  VK_CAPITAL, XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_MODE_JP_HALFWIDTH_ALPHANUMERIC, (IMECALLBACK)GFxIMEManagerXBox360::ChangeInputMode },                            //for Japanese
    { MODE_INPUT | MODE_JP,  VK_KANJI,   XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                            //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_INPUT | MODE_JP,  VK_KANJI,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },     //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_INPUT | MODE_JP,  VK_OEM_3,   XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                            //for Japanese, special conversion, when dashboard is set to non-Japanese
    { MODE_INPUT | MODE_JP,  VK_OEM_3,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },     //for Japanese, special conversion, when dashboard is set to non-Japanese

    { MODE_INPUT | MODE_KR,  VK_HANGUL,  XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },

    { MODE_INPUT | MODE_TC,  VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },    //for Bopomofo
    { MODE_INPUT | MODE_TC,  VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN,  XINPUT_KEYSTROKE_ALT,   XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },      //for , Bopomofo

    // Convert mode
    { MODE_CONVERT | MODE_JKT, VK_SHIFT,  XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT | XINPUT_KEYSTROKE_SHIFT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::SwitchLanguage },  //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JKT, VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::SwitchLanguage },  //for Japanese, Hangul, Bopomofo

    { MODE_CONVERT | MODE_JP | MODE_TC, VK_RETURN,XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_COMPLETE, (IMECALLBACK)GFxIMEManagerXBox360::Complete },//for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP,  VK_SEPARATOR, XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_COMPLETE, (IMECALLBACK)GFxIMEManagerXBox360::Complete },     //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP,  VK_RIGHT, XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_RIGHT, (IMECALLBACK)GFxIMEManagerXBox360::ChangeFocus },         //for Japanese, Hangul
    { MODE_CONVERT | MODE_JP,  VK_LEFT,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_LEFT, (IMECALLBACK)GFxIMEManagerXBox360::ChangeFocus },          //for Japanese, Hangul
    { MODE_CONVERT | MODE_JP,  VK_RIGHT, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0,   XIME_KEY_RIGHT, (IMECALLBACK)GFxIMEManagerXBox360::ChangeClauseLength },          //for Japanese, Hangul
    { MODE_CONVERT | MODE_JP,  VK_LEFT,  XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0,   XIME_KEY_LEFT, (IMECALLBACK)GFxIMEManagerXBox360::ChangeClauseLength },           //for Japanese, Hangul
    { MODE_CONVERT | MODE_JP,  VK_ESCAPE,XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_FLUSH, (IMECALLBACK)GFxIMEManagerXBox360::Revert },              //for Japanese
    { MODE_CONVERT | MODE_JP,  VK_BACK,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_FLUSH, (IMECALLBACK)GFxIMEManagerXBox360::Revert },              //for Japanese
    { MODE_CONVERT | MODE_JP,  VK_SPACE, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_NEXT, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },                      //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP,  VK_CONVERT, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_NEXT, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },                    //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP,  VK_CONVERT, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0, (XIMEKEY)XIME_INDEX_PREV, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },           //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_UP,    XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_PREV, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },             //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP,  VK_SPACE, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0,   (XIMEKEY)XIME_INDEX_PREV, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },           //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_DOWN,  XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_NEXT, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },             //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_PRIOR, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_TOP_OF_PREV_PAGE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate }, //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NEXT,  XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_TOP_OF_NEXT_PAGE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate }, //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_HOME,  XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_TOP, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },              //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_END,   XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_END, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },              //for Japanese, Bopomofo
    { MODE_CONVERT | MODE_JP,  VK_NONCONVERT, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_HIRAGANA_KATAKANA_TOGGLE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },            //for Japanese
    { MODE_CONVERT | MODE_JP,  VK_F6,    XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_HIRAGANA_CANDIDATE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },        //for Japanese
    { MODE_CONVERT | MODE_JP,  VK_F7,    XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_KATAKANA_CANDIDATE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },        //for Japanese
    { MODE_CONVERT | MODE_JP,  L'U',     XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   (XIMEKEY)XIME_INDEX_HIRAGANA_CANDIDATE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },      //for Japanese
    { MODE_CONVERT | MODE_JP,  L'I',     XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   (XIMEKEY)XIME_INDEX_KATAKANA_CANDIDATE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },      //for Japanese
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD0, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD1, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD2, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD3, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD4, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD5, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD6, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD7, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD8, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_NUMPAD9, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP,  VK_KANJI, XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                                //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_CONVERT | MODE_JP,  VK_KANJI, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },         //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_CONVERT | MODE_JP,  VK_OEM_3, XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                                //for Japanese, special conversion, when dashboard is set to English
    { MODE_CONVERT | MODE_JP,  VK_OEM_3, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },         //for Japanese, special conversion, when dashboard is set to English
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_0, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_1, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_2, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_3, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_4, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_5, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_6, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_7, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_8, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion
    { MODE_CONVERT | MODE_JP | MODE_TC, VK_9, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_CURRENT_PAGE_OFFSET, (IMECALLBACK)GFxIMEManagerXBox360::HandleNumpadConversion },          //for Japanese, special conversion

    { MODE_CONVERT | MODE_TC,  VK_SPACE, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },
    { MODE_CONVERT | MODE_TC,  VK_ESCAPE,XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_FLUSH, NULL },                                   //for Bopomofo
    { MODE_CONVERT | MODE_TC,  VK_BACK,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_UNICODE, (IMECALLBACK)GFxIMEManagerXBox360::NothingToDo },       //for Bopomofo
    { MODE_CONVERT | MODE_TC,  VK_SPACE, XINPUT_KEYSTROKE_KEYDOWN, 0, (XIMEKEY)XIME_INDEX_TOP_OF_NEXT_PAGE, (IMECALLBACK)GFxIMEManagerXBox360::SetCandidate },          //for Japanese, Bopomofo


    // All conversion mode
    { MODE_ALLCONVERT | MODE_JKT, VK_SHIFT,  XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT | XINPUT_KEYSTROKE_SHIFT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::SwitchLanguage },  //for Japanese, Bopomofo

    { MODE_ALLCONVERT | MODE_JP,  VK_RETURN,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_COMPLETE, (IMECALLBACK)GFxIMEManagerXBox360::Complete },        //for Japanese, Bopomofo
    { MODE_ALLCONVERT | MODE_JP,  VK_SEPARATOR,XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_COMPLETE, (IMECALLBACK)GFxIMEManagerXBox360::Complete },       //for Japanese, Bopomofo
    { MODE_ALLCONVERT | MODE_JP,  VK_ESCAPE,  XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_FLUSH, (IMECALLBACK)GFxIMEManagerXBox360::AllRevert },          //for Japanese, Bopomofo
    { MODE_ALLCONVERT | MODE_JP,  VK_BACK,    XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_FLUSH, (IMECALLBACK)GFxIMEManagerXBox360::AllRevert },          //for Japanese, Bopomofo
    { MODE_ALLCONVERT | MODE_JP,  VK_F6,      XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_HIRAGANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },          //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  VK_F7,      XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_KATAKANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },          //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  VK_F9,      XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_ALPHANUMERIC_FULL_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },           //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  VK_F10,     XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_ALL_ALPHANUMERIC_HALF_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },           //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  L'U',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_HIRAGANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },   //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  L'I',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_KATAKANA, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },   //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  L'P',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_ALPHANUMERIC_FULL_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },          //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  L'O',       XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0,   XIME_KEY_ALL_ALPHANUMERIC_HALF_WIDTH, (IMECALLBACK)GFxIMEManagerXBox360::AllConvert },          //for Japanese, special conversion
    { MODE_ALLCONVERT | MODE_JP,  VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },         //for Japanese, Bopomofo
    { MODE_ALLCONVERT | MODE_JP,  VK_CONVERT, XINPUT_KEYSTROKE_KEYDOWN, 0, XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },         //for Japanese, Bopomofo
    { MODE_ALLCONVERT | MODE_JP,  VK_CONVERT, XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_SHIFT, 0, XIME_KEY_CONVERT, (IMECALLBACK)GFxIMEManagerXBox360::Convert },         //for Japanese, Bopomofo

    { MODE_ALLCONVERT | MODE_JP,  VK_KANJI,   XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                         //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_ALLCONVERT | MODE_JP,  VK_KANJI,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },  //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_ALLCONVERT | MODE_JP,  VK_OEM_3,   XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                         //for Japanese, special conversion, when dashboard is set to non-Japanese
    { MODE_ALLCONVERT | MODE_JP,  VK_OEM_3,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },  //for Japanese, special conversion, when dashboard is set to non-Japanese

    // GFxIMEManagerXBox360 Off mode
    { MODE_OFF | MODE_JKT, VK_SHIFT,  XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT | XINPUT_KEYSTROKE_SHIFT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::SwitchLanguage },  //for Japanese, Hangul, Bopomofo
    { MODE_OFF | MODE_JP, VK_KANJI,   XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                                 //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_OFF | MODE_JP, VK_KANJI,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },          //for Japanese, special conversion, when dashboard is set to Japanese
    { MODE_OFF | MODE_JP, VK_OEM_3,   XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },                                 //for Japanese, special conversion, when dashboard is set to non_Japanese
    { MODE_OFF | MODE_JP, VK_OEM_3,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_ALT, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },          //for Japanese, special conversion, when dashboard is set to non_Japanese

    { MODE_OFF | MODE_KR, VK_HANGUL,  XINPUT_KEYSTROKE_KEYDOWN, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },

    { MODE_OFF | MODE_JKT, VK_SPACE,   XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_CTRL, 0, 0, (IMECALLBACK)GFxIMEManagerXBox360::ToggleIME },

};

//---------------------------------------------------------------
//   Lookup Table For Japanese (Hiragana character) Non-alphabet characters
//---------------------------------------------------------------
HID_LOOKUP_TABLE JapaneseHiraganaNonAlphabetic[] = {
    { 0x1e,   0x38,   0,      0 },
    /*HID_USAGE_INDEX_KEYBOARD_ONE*/          { 0x306C, 0x306C, 0x30CC, 0x30CC }, // nu, nu
    /*HID_USAGE_INDEX_KEYBOARD_TWO*/          { 0x3075, 0x3075, 0x30D5, 0x30D5 }, // fu, fu
    /*HID_USAGE_INDEX_KEYBOARD_THREE*/        { 0x3042, 0x3041, 0x30A2, 0x30A1 }, // a, a (small)
    /*HID_USAGE_INDEX_KEYBOARD_FOUR*/         { 0x3046, 0x3045, 0x30A6, 0x30A5 }, // u, u (small)
    /*HID_USAGE_INDEX_KEYBOARD_FIVE*/         { 0x3048, 0x3047, 0x30A8, 0x30A7 }, // e, e (small)
    /*HID_USAGE_INDEX_KEYBOARD_SIX*/          { 0x304A, 0x3049, 0x30AA, 0x30A9 }, // o, o (small)
    /*HID_USAGE_INDEX_KEYBOARD_SEVEN*/        { 0x3084, 0x3083, 0x30E4, 0x30E3 }, // ya, ya (small)
    /*HID_USAGE_INDEX_KEYBOARD_EIGHT*/        { 0x3086, 0x3085, 0x30E6, 0x30E5 }, // yu, yu (small)
    /*HID_USAGE_INDEX_KEYBOARD_NINE*/         { 0x3088, 0x3087, 0x30E8, 0x30E7 }, // yo, yo (small)
    /*HID_USAGE_INDEX_KEYBOARD_ZERO*/         { 0x308F, 0x3092, 0x30EF, 0x30F2 }, // wa, o
    /*HID_USAGE_INDEX_KEYBOARD_RETURN*/       { 0x000A, 0x000A, 0x000A, 0x000A }, //LF,LF
    /*HID_USAGE_INDEX_KEYBOARD_ESCAPE*/       { 0x001B, 0x001B, 0x001B, 0x001B }, //ESC,ESC
    /*HID_USAGE_INDEX_KEYBOARD_BACKSPACE*/    { 0x0008, 0x0008, 0x0008, 0x0008 }, //BS
    /*HID_USAGE_INDEX_KEYBOARD_TAB*/          { 0x0000, 0x0000, 0x0000, 0x0000 }, //TAB is not supported
    /*HID_USAGE_INDEX_KEYBOARD_SPACEBAR*/     { 0x0020, 0x0020, 0x0020, 0x0020 }, //SPACE,SPACE
    /*HID_USAGE_INDEX_KEYBOARD_MINUS*/        { 0x307B, 0x307B, 0x30DB, 0x30DB }, // ho, ho
    /*HID_USAGE_INDEX_KEYBOARD_EQUALS*/       { 0x3078, 0x3078, 0x30D8, 0x30D8 }, // he, he
    /*HID_USAGE_INDEX_KEYBOARD_OPEN_BRACE*/   { 0x309B, 0x309B, 0x309B, 0x309B }, // voiced sound mark, voiced sound mark
    /*HID_USAGE_INDEX_KEYBOARD_CLOSE_BRACE*/  { 0x309C, 0x300C, 0x309C, 0x300C }, // semi-voiced sound mark, left corner bracket
    /*HID_USAGE_INDEX_KEYBOARD_BACKSLASH*/    { 0x0000, 0x0000, 0x0000, 0x0000 }, // no character, no character
    /*HID_USAGE_INDEX_KEYBOARD_NON_US_TILDE*/ { 0x3080, 0x300D, 0x30E0, 0x300D }, // mu, right corner bracket
    /*HID_USAGE_INDEX_KEYBOARD_COLON*/        { 0x308C, 0x308C, 0x30EC, 0x30EC }, // re, re
    /*HID_USAGE_INDEX_KEYBOARD_QUOTE*/        { 0x3051, 0x3051, 0x30B1, 0x30B1 }, // ke, ke
    /*HID_USAGE_INDEX_KEYBOARD_TILDE*/        { 0x0000, 0x0000, 0x0000, 0x0000 }, // no character, no character (Japanese/English toggle)
    /*HID_USAGE_INDEX_KEYBOARD_COMMA*/        { 0x306D, 0x3001, 0x30CD, 0x3001 }, // ne, ideographic comma
    /*HID_USAGE_INDEX_KEYBOARD_PERIOD*/       { 0x308B, 0x3002, 0x30EB, 0x3002 }, // ru,ideographic full stop
    /*HID_USAGE_INDEX_KEYBOARD_QUESTION*/     { 0x3081, 0x30FB, 0x30E1, 0x30FB }  // me, middle dot
};

//---------------------------------------------------------------
//   Lookup Table For Japanese (Hiragana character) alphabet characters
//---------------------------------------------------------------
HID_LOOKUP_TABLE JapaneseHiraganaAlphabetic[] = {
    { 0x04,   0x1d,   0,      0 },
    /*HID_USAGE_INDEX_KEYBOARD_aA*/           { 0x3061, 0x3061, 0x30C1, 0x30C1 },  // chi, chi
    /*HID_USAGE_INDEX_KEYBOARD_bB*/           { 0x3053, 0x3053, 0x30B3, 0x30B3 },  // ko, ko
    /*HID_USAGE_INDEX_KEYBOARD_cC*/           { 0x305D, 0x305D, 0x30BD, 0x30BD },  // so, so
    /*HID_USAGE_INDEX_KEYBOARD_dD*/           { 0x3057, 0x3057, 0x30B7, 0x30B7 },  // shi, shi
    /*HID_USAGE_INDEX_KEYBOARD_eE*/           { 0x3044, 0x3043, 0x30A4, 0x30A3 },  // i, i (small)
    /*HID_USAGE_INDEX_KEYBOARD_fF*/           { 0x306F, 0x306F, 0x30CF, 0x30CF },  // ha, ha
    /*HID_USAGE_INDEX_KEYBOARD_gG*/           { 0x304D, 0x304D, 0x30AD, 0x30AD },  // ki, ki
    /*HID_USAGE_INDEX_KEYBOARD_hH*/           { 0x304F, 0x304F, 0x30AF, 0x30AF },  // ku, ku
    /*HID_USAGE_INDEX_KEYBOARD_iI*/           { 0x306B, 0x306B, 0x30CB, 0x30CB },  // ni, ni
    /*HID_USAGE_INDEX_KEYBOARD_jJ*/           { 0x307E, 0x307E, 0x30DE, 0x30DE },  // ma, ma
    /*HID_USAGE_INDEX_KEYBOARD_kK*/           { 0x306E, 0x306E, 0x30CE, 0x30CE }, // no, no
    /*HID_USAGE_INDEX_KEYBOARD_lL*/           { 0x308A, 0x308A, 0x30EA, 0x30EA }, // ri, ri
    /*HID_USAGE_INDEX_KEYBOARD_mM*/           { 0x3082, 0x3082, 0x30E2, 0x30E2 }, // mo, mo
    /*HID_USAGE_INDEX_KEYBOARD_nN*/           { 0x307F, 0x307F, 0x30DF, 0x30DF }, // mi, mi
    /*HID_USAGE_INDEX_KEYBOARD_oO*/           { 0x3089, 0x3089, 0x30E9, 0x30E9 }, // ra,ra
    /*HID_USAGE_INDEX_KEYBOARD_pP*/           { 0x305B, 0x305B, 0x30BB, 0x30BB }, // se, se
    /*HID_USAGE_INDEX_KEYBOARD_qQ*/           { 0x305F, 0x305F, 0x30BF, 0x30BF }, // ta, ta
    /*HID_USAGE_INDEX_KEYBOARD_rR*/           { 0x3059, 0x3059, 0x30B9, 0x30B9 }, // su, su
    /*HID_USAGE_INDEX_KEYBOARD_sS*/           { 0x3068, 0x3068, 0x30C8, 0x30C8 }, // to, to
    /*HID_USAGE_INDEX_KEYBOARD_tT*/           { 0x304B, 0x304B, 0x30AB, 0x30AB }, // ka, ka
    /*HID_USAGE_INDEX_KEYBOARD_uU*/           { 0x306A, 0x306A, 0x30CA, 0x30CA }, // na, na
    /*HID_USAGE_INDEX_KEYBOARD_vV*/           { 0x3072, 0x3072, 0x30D2, 0x30D2 }, // hi, hi
    /*HID_USAGE_INDEX_KEYBOARD_wW*/           { 0x3066, 0x3066, 0x30C6, 0x30C6 }, // te, te
    /*HID_USAGE_INDEX_KEYBOARD_xX*/           { 0x3055, 0x3055, 0x30B5, 0x30B5 }, // sa, sa
    /*HID_USAGE_INDEX_KEYBOARD_yY*/           { 0x3093, 0x3093, 0x30F3, 0x30F3 }, // n, n
    /*HID_USAGE_INDEX_KEYBOARD_zZ*/           { 0x3064, 0x3063, 0x30C4, 0x30C3 }  // tsu, tsu (small)
};


//---------------------------------------------------------------
//   Lookup Table For Korean Non-alphabet characters
//---------------------------------------------------------------
HID_LOOKUP_TABLE KoreanNonAlphabetic[] = {
    { 0x1e,   0x38,   0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_ONE*/          { 0x0031, 0x0021, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_TWO*/          { 0x0032, 0x0040, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_THREE*/        { 0x0033, 0x0023, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_FOUR*/         { 0x0034, 0x0024, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_FIVE*/         { 0x0035, 0x0025, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_SIX*/          { 0x0036, 0x005E, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_SEVEN*/        { 0x0037, 0x0026, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_EIGHT*/        { 0x0038, 0x002A, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_NINE*/         { 0x0039, 0x0028, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_ZERO*/         { 0x0030, 0x0029, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_RETURN*/       { 0x000A, 0x000A, 0, 0 }, // LF,LF
    /*HID_USAGE_INDEX_KEYBOARD_ESCAPE*/       { 0x001B, 0x001B, 0, 0 }, // ESC,ESC
    /*HID_USAGE_INDEX_KEYBOARD_BACKSPACE*/    { 0x0008, 0x0008, 0, 0 }, // BS
    /*HID_USAGE_INDEX_KEYBOARD_TAB*/          { 0x0000, 0x0000, 0, 0 }, // TAB is not supported
    /*HID_USAGE_INDEX_KEYBOARD_SPACEBAR*/     { 0x0020, 0x0020, 0, 0 }, // SPACE,SPACE
    /*HID_USAGE_INDEX_KEYBOARD_MINUS*/        { 0x002D, 0x005F, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_EQUALS*/       { 0x003D, 0x002B, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_OPEN_BRACE*/   { 0x005B, 0x007B, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_CLOSE_BRACE*/  { 0x005D, 0x007D, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_BACKSLASH*/    { 0x005C, 0x007C, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_NON_US_TILDE*/ { 0x0000, 0x0000, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_COLON*/        { 0x003B, 0x003A, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_QUOTE*/        { 0x0027, 0x0022, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_TILDE*/        { 0x0060, 0x007E, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_COMMA*/        { 0x002C, 0x003C, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_PERIOD*/       { 0x002E, 0x003E, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_QUESTION*/     { 0x002F, 0x003F, 0, 0 }, // 
};

//---------------------------------------------------------------
//   Lookup Table For Korean alphabet characters
//---------------------------------------------------------------
HID_LOOKUP_TABLE KoreanAlphabetic[] = {
    { 0x04,   0x1d,   0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_aA*/           { 0x3141, 0x3141, 0, 0 }, // Mieum, Mieum
    /*HID_USAGE_INDEX_KEYBOARD_bB*/           { 0x3160, 0x3160, 0, 0 }, // Yu, Yu
    /*HID_USAGE_INDEX_KEYBOARD_cC*/           { 0x314A, 0x314A, 0, 0 }, // Chieuch, Chieuch
    /*HID_USAGE_INDEX_KEYBOARD_dD*/           { 0x3147, 0x3147, 0, 0 }, // leung, leung
    /*HID_USAGE_INDEX_KEYBOARD_eE*/           { 0x3137, 0x3138, 0, 0 }, // Tikeut, Ssangtikeut
    /*HID_USAGE_INDEX_KEYBOARD_fF*/           { 0x3139, 0x3139, 0, 0 }, // Rieul, Rieul
    /*HID_USAGE_INDEX_KEYBOARD_gG*/           { 0x314E, 0x314E, 0, 0 }, // Hieuh, Hieuh
    /*HID_USAGE_INDEX_KEYBOARD_hH*/           { 0x3157, 0x3157, 0, 0 }, // O, O
    /*HID_USAGE_INDEX_KEYBOARD_iI*/           { 0x3151, 0x3151, 0, 0 }, // Ya, Ya
    /*HID_USAGE_INDEX_KEYBOARD_jJ*/           { 0x3153, 0x3153, 0, 0 }, // Eo, Eo
    /*HID_USAGE_INDEX_KEYBOARD_kK*/           { 0x314F, 0x314F, 0, 0 }, // A, A
    /*HID_USAGE_INDEX_KEYBOARD_lL*/           { 0x3163, 0x3163, 0, 0 }, // I, I
    /*HID_USAGE_INDEX_KEYBOARD_mM*/           { 0x3161, 0x3161, 0, 0 }, // Eu, Eu
    /*HID_USAGE_INDEX_KEYBOARD_nN*/           { 0x315C, 0x315C, 0, 0 }, // U, U
    /*HID_USAGE_INDEX_KEYBOARD_oO*/           { 0x3150, 0x3152, 0, 0 }, // Ae, Yae
    /*HID_USAGE_INDEX_KEYBOARD_pP*/           { 0x3154, 0x3156, 0, 0 }, // E, Ye
    /*HID_USAGE_INDEX_KEYBOARD_qQ*/           { 0x3142, 0x3143, 0, 0 }, // Pieup, Ssangpieup
    /*HID_USAGE_INDEX_KEYBOARD_rR*/           { 0x3131, 0x3132, 0, 0 }, // Kiyeok, Ssangkiyeok
    /*HID_USAGE_INDEX_KEYBOARD_sS*/           { 0x3134, 0x3134, 0, 0 }, // Nieun, Nieun
    /*HID_USAGE_INDEX_KEYBOARD_tT*/           { 0x3145, 0x3146, 0, 0 }, // Sios, Ssangsios
    /*HID_USAGE_INDEX_KEYBOARD_uU*/           { 0x3155, 0x3155, 0, 0 }, // Yeo, Yeo
    /*HID_USAGE_INDEX_KEYBOARD_vV*/           { 0x314D, 0x314D, 0, 0 }, // Phieuph, Phieuph
    /*HID_USAGE_INDEX_KEYBOARD_wW*/           { 0x3148, 0x3149, 0, 0 }, // Cieuc, Ssangcieuc
    /*HID_USAGE_INDEX_KEYBOARD_xX*/           { 0x314C, 0x314C, 0, 0 }, // Thieuth, Thieuth
    /*HID_USAGE_INDEX_KEYBOARD_yY*/           { 0x315B, 0x315B, 0, 0 }, // Yo, Yo
    /*HID_USAGE_INDEX_KEYBOARD_zZ*/           { 0x314B, 0x314B, 0, 0 }  // Khieukh, Khieukh
};

//---------------------------------------------------------------
//   Lookup Table For 10 key
//---------------------------------------------------------------
WCHAR TenKeyTable[] = {
    // From 0x54 to 0x63
    /*HID_USAGE_INDEX_TENKEY_SEPARATOR*/      L'/',
    /*HID_USAGE_INDEX_TENKEY_**/              L'*',
    /*HID_USAGE_INDEX_TENKEY_-*/              L'-',
    /*HID_USAGE_INDEX_TENKEY_+*/              L'+',
    /*HID_USAGE_INDEX_TENKEY_ENTER*/          0x0013,
    /*HID_USAGE_INDEX_TENKEY_1*/              L'1',
    /*HID_USAGE_INDEX_TENKEY_2*/              L'2',
    /*HID_USAGE_INDEX_TENKEY_3*/              L'3',
    /*HID_USAGE_INDEX_TENKEY_4*/              L'4',
    /*HID_USAGE_INDEX_TENKEY_5*/              L'5',
    /*HID_USAGE_INDEX_TENKEY_6*/              L'6',
    /*HID_USAGE_INDEX_TENKEY_7*/              L'7',
    /*HID_USAGE_INDEX_TENKEY_8*/              L'8',
    /*HID_USAGE_INDEX_TENKEY_9*/              L'9',
    /*HID_USAGE_INDEX_TENKEY_0*/              L'0',
    /*HID_USAGE_INDEX_TENKEY_.*/              L'.'
};


//---------------------------------------------------------------
//   Lookup Table For ChineseTraditional (Bopomofo) Non-alphabet characters
//---------------------------------------------------------------
HID_LOOKUP_TABLE ChineseTraditionalBopomofoNonAlphabetic[] = {
    { 0x1e,  0x38,   0,      0 },
    /*HID_USAGE_INDEX_KEYBOARD_ONE*/          { 0x3105, 0x3105, 0, 0 }, // Half pinyin B
    /*HID_USAGE_INDEX_KEYBOARD_TWO*/          { 0x3109, 0x3109, 0, 0 }, // Half pinyin D
    /*HID_USAGE_INDEX_KEYBOARD_THREE*/        { 0x02C7, 0x02C7, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_FOUR*/         { 0x02CB, 0x02CB, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_FIVE*/         { 0x3113, 0x3113, 0, 0 }, //
    /*HID_USAGE_INDEX_KEYBOARD_SIX*/          { 0x02CA, 0x02CA, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_SEVEN*/        { 0x02D9, 0x02D9, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_EIGHT*/        { 0x311A, 0x311A, 0, 0 }, //
    /*HID_USAGE_INDEX_KEYBOARD_NINE*/         { 0x311E, 0x311E, 0, 0 }, //
    /*HID_USAGE_INDEX_KEYBOARD_ZERO*/         { 0x3122, 0x3122, 0, 0 }, //
    /*HID_USAGE_INDEX_KEYBOARD_RETURN*/       { 0x000A, 0x000A, 0, 0 }, //LF,LF
    /*HID_USAGE_INDEX_KEYBOARD_ESCAPE*/       { 0x001B, 0x001B, 0, 0 }, //ESC,ESC
    /*HID_USAGE_INDEX_KEYBOARD_BACKSPACE*/    { 0x0008, 0x0008, 0, 0 }, //BS
    /*HID_USAGE_INDEX_KEYBOARD_TAB*/          { 0x0000, 0x0000, 0, 0 }, //TAB is not supported
    /*HID_USAGE_INDEX_KEYBOARD_SPACEBAR*/     { 0x0020, 0x0020, 0, 0 }, //SPACE,SPACE
    /*HID_USAGE_INDEX_KEYBOARD_MINUS*/        { 0x3126, 0x3126, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_EQUALS*/       { 0x003D, 0x002B, 0, 0 }, // '=', '+'
    /*HID_USAGE_INDEX_KEYBOARD_OPEN_BRACE*/   { 0x005B, 0x007B, 0, 0 }, // '[', '{'
    /*HID_USAGE_INDEX_KEYBOARD_CLOSE_BRACE*/  { 0x005D, 0x007D, 0, 0 }, // ']', '}'
    /*HID_USAGE_INDEX_KEYBOARD_BACKSLASH*/    { 0x005C, 0x007C, 0, 0 }, // Backslash, Vertical Line
    /*HID_USAGE_INDEX_KEYBOARD_NON_US_TILDE*/ { 0x0000, 0x0000, 0, 0 }, // Not on US keyboard
    /*HID_USAGE_INDEX_KEYBOARD_COLON*/        { 0x3124, 0x3124, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_QUOTE*/        { 0x0027, 0x0022, 0, 0 }, // ''', '"'
    /*HID_USAGE_INDEX_KEYBOARD_TILDE*/        { 0x0060, 0x007E, 0, 0 }, // '`', '~'
    /*HID_USAGE_INDEX_KEYBOARD_COMMA*/        { 0x311D, 0x311D, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_PERIOD*/       { 0x3121, 0x3121, 0, 0 }, // 
    /*HID_USAGE_INDEX_KEYBOARD_QUESTION*/     { 0x3125, 0x3125, 0, 0 }  // 
};

//---------------------------------------------------------------
//   Lookup Table For Chinese (Bopomofo) alphabet characters
//---------------------------------------------------------------
HID_LOOKUP_TABLE ChineseTraditionalBopomofoAlphabetic[] = {
    { 0x04,   0x1d,  0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_aA*/           { 0x3107, 0x3107, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_bB*/           { 0x3116, 0x3116, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_cC*/           { 0x310F, 0x310F, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_dD*/           { 0x310E, 0x310E, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_eE*/           { 0x310D, 0x310D, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_fF*/           { 0x3111, 0x3111, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_gG*/           { 0x3115, 0x3115, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_hH*/           { 0x3118, 0x3118, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_iI*/           { 0x311B, 0x311B, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_jJ*/           { 0x3128, 0x3128, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_kK*/           { 0x311C, 0x311C, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_lL*/           { 0x3120, 0x3120, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_mM*/           { 0x3129, 0x3129, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_nN*/           { 0x3119, 0x3119, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_oO*/           { 0x311F, 0x311F, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_pP*/           { 0x3123, 0x3123, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_qQ*/           { 0x3106, 0x3106, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_rR*/           { 0x3110, 0x3110, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_sS*/           { 0x310B, 0x310B, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_tT*/           { 0x3114, 0x3114, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_uU*/           { 0x3127, 0x3127, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_vV*/           { 0x3112, 0x3112, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_wW*/           { 0x310A, 0x310A, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_xX*/           { 0x310C, 0x310C, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_yY*/           { 0x3117, 0x3117, 0, 0 },
    /*HID_USAGE_INDEX_KEYBOARD_zZ*/           { 0x3108, 0x3108, 0, 0 },
};

XIMEPOS_LOOKUP_TABLE XimeposTable[] = 
{
    XIMEPOS_NOUN,                           L"XIMEPOS_NOUN",
    XIMEPOS_PSEUDO_NOUN_SURU,               L"XIMEPOS_PSEUDO_NOUN_SURU",
    XIMEPOS_PSEUDO_NOUN_ZURU,               L"XIMEPOS_PSEUDO_NOUN_ZURU",
    XIMEPOS_ADJECTIVAL_NOUN,                L"XIMEPOS_ADJECTIVAL_NOUN",
    XIMEPOS_ADVERBIAL_NOUN,                 L"XIMEPOS_ADVERBIAL_NOUN",
    XIMEPOS_PSEUDO_ADJECTIVAL_NOUN,         L"XIMEPOS_PSEUDO_ADJECTIVAL_NOUN",
    XIMEPOS_HUMAN_NAME,                     L"XIMEPOS_HUMAN_NAME",
    XIMEPOS_FAMILY_NAME,                    L"XIMEPOS_FAMILY_NAME",
    XIMEPOS_GIVEN_NAME,                     L"XIMEPOS_GIVEN_NAME",
    XIMEPOS_PLACE_NAME,                     L"XIMEPOS_PLACE_NAME",
    XIMEPOS_PROPER_NOUN,                    L"XIMEPOS_PROPER_NOUN",
    XIMEPOS_VERB_5THSTEP_A_WA,              L"XIMEPOS_VERB_5THSTEP_A_WA",
    XIMEPOS_VERB_5THSTEP_KA,                L"XIMEPOS_VERB_5THSTEP_KA",
    XIMEPOS_VERB_5THSTEP_GA,                L"XIMEPOS_VERB_5THSTEP_GA",
    XIMEPOS_VERB_5THSTEP_SA,                L"XIMEPOS_VERB_5THSTEP_SA",
    XIMEPOS_VERB_5THSTEP_TA,                L"XIMEPOS_VERB_5THSTEP_TA",
    XIMEPOS_VERB_5THSTEP_NA,                L"XIMEPOS_VERB_5THSTEP_NA",
    XIMEPOS_VERB_5THSTEP_BA,                L"XIMEPOS_VERB_5THSTEP_BA",
    XIMEPOS_VERB_5THSTEP_MA,                L"XIMEPOS_VERB_5THSTEP_MA",
    XIMEPOS_VERB_5THSTEP_RA,                L"XIMEPOS_VERB_5THSTEP_RA",
    XIMEPOS_VERB_1STSTEP,                   L"XIMEPOS_VERB_1STSTEP",
    XIMEPOS_ADJECTIVE,                      L"XIMEPOS_ADJECTIVE",
    XIMEPOS_NOMINAL_ADJECTIVE,              L"XIMEPOS_NOMINAL_ADJECTIVE",
    XIMEPOS_ADVERB,                         L"XIMEPOS_ADVERB",
    XIMEPOS_ADNOUN,                         L"XIMEPOS_ADNOUN",
    XIMEPOS_CONJUNCTION,                    L"XIMEPOS_CONJUNCTION",
    XIMEPOS_INTERJECTION,                   L"XIMEPOS_INTERJECTION",
    XIMEPOS_PREFIX,                         L"XIMEPOS_PREFIX",
    XIMEPOS_SUFFIX,                         L"XIMEPOS_SUFFIX",
    XIMEPOS_SUFFIX_FOR_NUMERALS,            L"XIMEPOS_SUFFIX_FOR_NUMERALS",
    XIMEPOS_SINGLE_KANJI,                   L"XIMEPOS_SINGLE_KANJI",
    XIMEPOS_FACE_MARK,                      L"XIMEPOS_FACE_MARK",
    XIMEPOS_IDIOM,                          L"XIMEPOS_IDIOM",
};


