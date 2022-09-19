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
      return "Gasoline";
    case (0x02):
      return "Methanol";
    case (0x03):
      return "Ethanol";
    case (0x04):
      return "Diesel";
    case (0x05):
      return "LPG";
    case (0x06):
      return "CNG";
    case (0x07):
      return "Propane";
    case (0x08):
      return "Electric";
    case (0x09):
      return "Bifuel Gasoline";
    case (0x0A):
      return "Bifuel Methanol";
    case (0x0B):
      return "Bifuel Ethanol";
    case (0x0C):
      return "Bifuel LPG";
    case (0x0D):
      return "Bifuel CNG";
    case (0x0E):
      return "Bifuel Propane";
    case (0x0F):
      return "Bifuel Electric";
    case (0x10):
      return "Bifuel Electric + Combustion";
    case (0x11):
      return "Hybrid Gasoline";
    case (0x12):
      return "Hybrid Ethanol";
    case (0x13):
      return "Hybrid Diesel";
    case (0x14):
      return "Hybrid Electric";
    case (0x15):
      return "Hybrid Electric + Combustion";
    case (0x16):
      return "Hybrid Regenerative";
    case (0x17):
      return "Bifuel Diesel";
    default:
      return "???";
  }
}