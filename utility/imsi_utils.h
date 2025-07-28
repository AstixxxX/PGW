#pragma once

#include <string>
#include <bitset>
#include <algorithm>

std::string imsiToBcd(const std::string& imsi) 
{
    std::bitset<4> a;
    std::string bcd;

    for (char ch : imsi)
    {
        a = ch;
        bcd += a.to_string();
    }

    return bcd;
}

bool valid_imsi(const std::string& imsi)
{
    return imsi.size() == 15 && std::all_of(imsi.begin(), imsi.end(), ::isdigit);
}

std::string bcdToImsi(const std::string& bcd)
{
    std::string imsi;
    std::string bin_digit;
    std::bitset<4> nibble;

    for (int i = 0; i < bcd.size(); i += 4)
    {
        bin_digit = bcd.substr(i, 4);
        std::bitset<4> nibble(bin_digit);
        imsi += std::to_string(nibble.to_ulong());
    }

    return imsi;
}