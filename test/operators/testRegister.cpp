#include "gtest/gtest.h"
#include "../../database/dbms.h"

int intValue = 20;
std::string strValue = "fish";

TEST(Register, ReadWrite)
{
    Register reg;

    reg.setInteger(intValue);
    ASSERT_EQ(intValue, reg.getInteger());
    
    reg.setString(strValue);
    ASSERT_STREQ(strValue.c_str(), reg.getString().c_str());
}