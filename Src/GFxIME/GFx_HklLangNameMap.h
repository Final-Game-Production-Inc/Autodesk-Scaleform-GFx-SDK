struct KeybCodes
{
    char* StatusSymbol;
    char* LocaleName;
    int   LocaleId;
    char* Unused;
};
static const KeybCodes KeybCodesList[] = {
    {"SQ", "Albanian"                               , 0x0000041c, NULL}, 
    {"AR", "Arabic (101)"                           , 0x00000401, NULL},
    {NULL, "Arabic (102) AZERTY"                    , 0x00020401, NULL},
    {NULL, "Arabic (102)"                           , 0x00010401, NULL},
    {NULL, "Armenian Eastern"                       , 0x0000042b, NULL},
    {NULL, "Armenian Western"                       , 0x0001042b, NULL},
    {NULL, "Azeri Cyrillic"                         , 0x0000082c, NULL},
    {NULL, "Azeri Latin"                            , 0x0000042c, NULL},
    {"BE", "Belarusian"                             , 0x00000423, NULL}, 
    {NULL, "Belgian (Comma)"                        , 0x0001080c, NULL},
    {NULL, "Belgian Dutch"                          , 0x00000813, NULL},
    {NULL, "Belgian French"                         , 0x0000080c, NULL}, 
    {NULL, "Bulgarian (Latin)"                      , 0x00010402, NULL}, 
    {"BG", "Bulgarian"                              , 0x00000402, NULL},
    {"CF", "Canadian French (Legacy)"               , 0x00000c0c, NULL}, 
    {NULL, "Canadian French"                        , 0x00001009, NULL}, 
    {NULL, "Canadian Multilingual Standard"         , 0x00011009, NULL},
    {NULL, "Chinese (Simplified) - MS-PinYin98"     , 0xE00E0804, NULL},
    {NULL, "Chinese (Simplified) - NeiMa"           , 0xE0050804, NULL},
    {NULL, "Chinese (Simplified) - QuanPin"         , 0xE0010804, NULL},
    {NULL, "Chinese (Simplified) - QuanPin"         , 0x08040804, NULL}, // Windows Vista
    {NULL, "Chinese (Simplified) - ShuangPin"       , 0xE0020804, NULL}, 
    {NULL, "Chinese (Simplified) - ShuangPin"       , 0x08040804, NULL}, // Windows Vista
    {NULL, "Chinese (Simplified) - US Keyboard"     , 0x00000804, NULL},
    {NULL, "Chinese (Simplified) - ZhengMa"         , 0xE0030804, NULL},
    {NULL, "Chinese (Traditional) - Alphanumeric"   , 0xE01F0404, NULL},
    {NULL, "Chinese (Traditional) - Array"          , 0xE0050404, NULL},
    {NULL, "Chinese (Traditional) - Big5 Code"      , 0xE0040404, NULL},
    {NULL, "Chinese (Traditional) - ChangJie"       , 0xE0020404, NULL},
    {NULL, "Chinese (Traditional) - DaYi"           , 0xE0060404, NULL},
    {NULL, "Chinese (Traditional) - New ChangJie"   , 0xE0090404, NULL},
    {NULL, "Chinese (Traditional) - New ChangJie"   , 0x04040404, NULL}, // Windows Vista
    {NULL, "Chinese (Traditional) - New Phonetic"   , 0xE0080404, NULL},
    {NULL, "Chinese (Traditional) - Phonetic"       , 0xE0010404, NULL},
    {NULL, "Chinese (Traditional) - Quick"          , 0xE0030404, NULL},
    {"ZH", "Chinese (Traditional) - US Keyboard"    , 0x00000404, NULL},
    {NULL, "Chinese (Traditional) - Unicode"        , 0xE0070404, NULL},
                                                                     
    {NULL, "Japanese Input System (MS-IME2000)"     , 0xE0010411, NULL},
    {"JA", "Japanese"                               , 0x00000411, NULL},
    {"JA", "Japanese"                               , 0x04110411, NULL}, // Windows Vista
    {NULL, "Japanese (106 keyboard)"                , 0x00000411, NULL}, /*
                                                                         double line */ /* is one of this two "Japanese Input System (MS-IME2000)" or
                                                                         "Japanese" */
    {NULL, "Japanese (pc98x1 keyboard)"             , 0x00000411, NULL}, /*
                                                                             double line */ /* is one of this two "Japanese Input System (MS-IME2000)" or
                                                                             "Japanese" */
    {"KK", "Kazakh"                                 , 0x0000043f, NULL},
    {NULL, "Korean(Hangul) (MS-IME98)"              , 0xE0010412, NULL},
    {"KO", "Korean(Hangul)"                         , 0x00000412, NULL},
    {"LA", "Latin American"                         , 0x0000080a, NULL},
    {NULL, "Russian (Typewriter)"                   , 0x00010419, NULL},
    {"RU", "Russian"                                , 0x00000419, NULL},
    {NULL, "Russian (KOI8-R)"                       , 0x00000419, NULL}, /* Double
                                                                   Line */ /* or maybe Typewriter?? */
    
    {NULL, NULL, 0,NULL}
};
