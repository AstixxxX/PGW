#include <gtest/gtest.h>
#include "../utility/imsi_utils.h"
#include <string>

TEST(ImsiConversionTest, ConvertsDigitsToBcd) 
{
    std::string imsi = "880000000000009";
    std::string expected = "100010000000000000000000000000000000000000000000000000001001";
    EXPECT_EQ(imsiToBcd(imsi), expected);
}

TEST(ImsiValidationTest, ValidatesCorrectImsi) 
{
    EXPECT_TRUE(valid_imsi("123456789012345"));
}

TEST(ImsiValidationTest, RejectsShortImsi) 
{
    EXPECT_FALSE(valid_imsi("12345678901234"));
}

TEST(ImsiValidationTest, RejectsNonDigitCharacters)
{
    EXPECT_FALSE(valid_imsi("12345678901234a"));
}

TEST(BcdConversionTest, ConvertsNibblesToImsi) 
{
    std::string bcd = "100010000000000000000000000000000000000000000000000000001001";
    std::string expected = "880000000000009";
    EXPECT_EQ(bcdToImsi(bcd), expected);
}