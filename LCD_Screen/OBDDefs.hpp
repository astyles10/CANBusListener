static const char* DetermineOBDStandard(const uint8_t inValue) {
  switch (inValue) {
    case (0x01):
      return "CARB OBD2";
    case (0x02):
      return "EPA OBD";
    case (0x03):
      return "OBD1+2";
    case (0x04):
      return "OBD1";
    case (0x05):
      return "NC";
    case (0x06):
      return "EOBD";
    case (0x07):
      return "EOBD+2";
    case (0x08):
      return "EOBD+1";
    case (0x09):
      return "EOBD+1+2";
    case (0x0A):
      return "JOBD";
    case (0x0B):
      return "JOBD+2";
    case (0x0C):
      return "JOBD+1";
    case (0x0D):
      return "JOBD+1+2";
    default:
      return "Other";
  }
}

static const char* DetermineFuelType(const uint8_t inValue) {
  switch (inValue) {
    case (0x00):
      return "NA";
    case (0x01):
      return "Gasoln";
    case (0x02):
      return "Mthnl";
    case (0x03):
      return "Ethnl";
    case (0x04):
      return "Diesel";
    case (0x05):
      return "LPG";
    case (0x06):
      return "CNG";
    case (0x07):
      return "Prpn";
    case (0x08):
      return "Elec";
    case (0x09):
      return "Bi Gasln";
    case (0x0A):
      return "Bi Mthln";
    case (0x0B):
      return "Bi Ethln";
    case (0x0C):
      return "Bi LPG";
    case (0x0D):
      return "Bi CNG";
    case (0x0E):
      return "Bi Prpn";
    case (0x0F):
      return "Bi Elec";
    case (0x10):
      return "Bi Elec+Cmb";
    case (0x11):
      return "Hy Gasln";
    case (0x12):
      return "Hy Ethnl";
    case (0x13):
      return "Hy Diesel";
    case (0x14):
      return "Hy Elec";
    case (0x15):
      return "Hy Elec+Cmg";
    case (0x16):
      return "Hy Regen";
    case (0x17):
      return "Bi Diesel";
    default:
      return "???";
  }
}