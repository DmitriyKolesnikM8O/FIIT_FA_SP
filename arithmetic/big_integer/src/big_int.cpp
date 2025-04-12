#include <vector>
#include <utility>
#include <iostream>
#include <concepts>
#include <string>
#include <sstream>
#include <algorithm>
#include "../include/big_int.h"

namespace
{
    constexpr unsigned long long BASE = 1ULL << (8 * sizeof(unsigned int));

    void optimise(std::vector<unsigned int, pp_allocator<unsigned int>>& digits)
    {
        while (digits.size() > 1 && digits.back() == 0)
        {
            digits.pop_back();
        }
    }

    bool is_zero(const std::vector<unsigned int, pp_allocator<unsigned int>>& digits)
    {
        return digits.size() == 1 && digits[0] == 0;
    }
}

big_int::multiplication_rule big_int::decide_mult(size_t rhs) const noexcept
{ 

    // return multiplication_rule::Karatsuba;
     

    const size_t threshold = 64;

    if (_digits.size() > threshold || rhs > threshold) {
        return multiplication_rule::Karatsuba;
    }
    return multiplication_rule::trivial;

}

big_int::division_rule big_int::decide_div(size_t) const noexcept
{

    return division_rule::trivial;
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int>>& digits, bool sign)
    : _sign(sign), _digits(digits)
{
    if (_digits.empty())
    {
        _digits.push_back(0);
    }
    optimise(_digits);
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>>&& digits, bool sign) noexcept
    : _sign(sign), _digits(std::move(digits))
{
    if (_digits.empty())
    {
        _digits.push_back(0);
    }
    optimise(_digits);
}

big_int::big_int(const std::string& num, unsigned int radix, pp_allocator<unsigned int> allocator)
    : _sign(true), _digits(allocator)
{
    if (num.empty())
    {
        _digits.push_back(0);
        return;
    }

    std::string number = num;
    bool is_negative = false;
    if (number[0] == '-')
    {
        is_negative = true;
        number = number.substr(1);
    }
    else if (number[0] == '+')
    {
        number = number.substr(1);
    }

    while (number.size() > 1 && number[0] == '0')
    {
        number = number.substr(1);
    }

    if (number.empty())
    {
        _digits.push_back(0);
        return;
    }

    _digits.push_back(0);
    for (char c : number)
    {
        if (!std::isdigit(c))
        {
            throw std::invalid_argument("Invalid character in number string");
        }
        unsigned int digit = c - '0';
        *this *= 10;
        *this += big_int(static_cast<long long>(digit), allocator);
    }

    _sign = !is_negative;

    if (is_zero(_digits))
    {
        _sign = true;
    }
}

big_int::big_int(pp_allocator<unsigned int> allocator)
    : _sign(true), _digits(allocator)
{
    _digits.push_back(0);
}

big_int::operator bool() const noexcept
{
    return !is_zero(_digits);
}

big_int& big_int::operator++() &
{
    *this += big_int(1, _digits.get_allocator());
    return *this;
}

big_int big_int::operator++(int)
{
    big_int temp(*this);
    ++(*this);
    return temp;
}

big_int& big_int::operator--() &
{
    *this -= big_int(1, _digits.get_allocator());
    return *this;
}

big_int big_int::operator--(int)
{
    big_int temp(*this);
    --(*this);
    return temp;
}

big_int& big_int::plus_assign(const big_int& other, size_t shift) &
{
    if (other._digits.size() == 1 && other._digits[0] == 0)
    {
        return *this;
    }

    if (_sign == other._sign)
    {
        size_t max_size = std::max(_digits.size(), other._digits.size() + shift);
        _digits.resize(max_size, 0);

        unsigned long long carry = 0;
        for (size_t i = 0; i < max_size; ++i)
        {
            unsigned long long sum = carry;
            if (i < _digits.size())
            {
                sum += _digits[i];
            }
            if (i >= shift && (i - shift) < other._digits.size())
            {
                sum += other._digits[i - shift];
            }
            _digits[i] = static_cast<unsigned int>(sum % BASE);
            carry = sum / BASE;
        }
        if (carry > 0)
        {
            _digits.push_back(static_cast<unsigned int>(carry));
        }
    }
    else
    {
        big_int temp(other);
        temp._sign = _sign;
        minus_assign(temp, shift);
    }

    optimise(_digits);
    return *this;
}

big_int& big_int::operator+=(const big_int& other) &
{
    return plus_assign(other, 0);
}

big_int& big_int::minus_assign(const big_int& other, size_t shift) &
{
    if (other._digits.size() == 1 && other._digits[0] == 0)
    {
        return *this;
    }

    if (_sign != other._sign)
    {
        big_int temp(other);
        temp._sign = _sign;
        return plus_assign(temp, shift);
    }

    big_int abs_this(*this);
    abs_this._sign = true;
    big_int abs_other(other);
    abs_other._sign = true;
    abs_other <<= shift;

    auto cmp = (abs_this <=> abs_other);
    bool result_sign = _sign;
    if (cmp == std::strong_ordering::less)
    {
        result_sign = !result_sign;
        std::vector<unsigned int, pp_allocator<unsigned int>> temp_digits = abs_other._digits;
        abs_other._digits = abs_this._digits;
        abs_this._digits = temp_digits;
    }

    size_t max_size = std::max(abs_this._digits.size(), abs_other._digits.size());
    std::vector<unsigned int, pp_allocator<unsigned int>> result(max_size, 0, _digits.get_allocator());
    long long borrow = 0;

    for (size_t i = 0; i < max_size; ++i)
    {
        long long diff = borrow;
        if (i < abs_this._digits.size())
        {
            diff += abs_this._digits[i];
        }
        if (i < abs_other._digits.size())
        {
            diff -= abs_other._digits[i];
        }
        if (diff < 0)
        {
            diff += BASE;
            borrow = -1;
        }
        else
        {
            borrow = 0;
        }
        result[i] = static_cast<unsigned int>(diff);
    }

    _digits = std::move(result);
    _sign = result_sign;
    optimise(_digits);

    if (is_zero(_digits))
    {
        _sign = true;
    }

    return *this;
}

big_int& big_int::operator-=(const big_int& other) &
{
    return minus_assign(other, 0);
}

big_int& big_int::multiply_assign(const big_int& other, multiplication_rule rule) &
{
    if (is_zero(other._digits))
    {
        _digits.clear();
        _digits.push_back(0);
        _sign = true;
        return *this;
    }

    if (is_zero(_digits))
    {
        return *this;
    }

    if (rule == multiplication_rule::Karatsuba) {
        size_t n = std::max(_digits.size(), other._digits.size());
        size_t m = n / 2 + (n % 2);
        
        if (n <= 4) {
            return multiply_assign(other, multiplication_rule::trivial);
        }
        
        std::vector<unsigned int, pp_allocator<unsigned int>> a_high(_digits.get_allocator());
        std::vector<unsigned int, pp_allocator<unsigned int>> a_low(_digits.get_allocator());
        std::vector<unsigned int, pp_allocator<unsigned int>> b_high(other._digits.get_allocator());
        std::vector<unsigned int, pp_allocator<unsigned int>> b_low(other._digits.get_allocator());
        
        if (_digits.size() <= m) {
            a_low = _digits;
            a_high.push_back(0);
        } else {
            a_low.assign(_digits.begin(), _digits.begin() + m);
            a_high.assign(_digits.begin() + m, _digits.end());
        }
        
        if (other._digits.size() <= m) {
            b_low = other._digits;
            b_high.push_back(0);
        } else {
            b_low.assign(other._digits.begin(), other._digits.begin() + m);
            b_high.assign(other._digits.begin() + m, other._digits.end());
        }
        
        optimise(a_low);
        optimise(a_high);
        optimise(b_low);
        optimise(b_high);
        
        big_int z0(a_low, true);
        z0.multiply_assign(big_int(b_low, true), multiplication_rule::trivial);
        
        big_int z2(a_high, true);
        z2.multiply_assign(big_int(b_high, true), multiplication_rule::trivial);
        
        big_int a_sum(a_low, true);
        a_sum += big_int(a_high, true);
        
        big_int b_sum(b_low, true);
        b_sum += big_int(b_high, true);
        
        big_int z1(a_sum);
        z1.multiply_assign(b_sum, multiplication_rule::trivial);
        z1 -= z0;
        z1 -= z2;
        
        std::vector<unsigned int, pp_allocator<unsigned int>> result(_digits.get_allocator());
        result.resize(n * 2, 0);
        
        for (size_t i = 0; i < z0._digits.size(); ++i) {
            result[i] = z0._digits[i];
        }
        
        for (size_t i = 0; i < z1._digits.size(); ++i) {
            unsigned long long temp = static_cast<unsigned long long>(result[i + m]) + z1._digits[i];
            result[i + m] = static_cast<unsigned int>(temp % BASE);
            if (temp >= BASE) {
                unsigned long long carry = temp / BASE;
                size_t j = i + m + 1;
                while (carry > 0 && j < result.size()) {
                    unsigned long long next = static_cast<unsigned long long>(result[j]) + carry;
                    result[j] = static_cast<unsigned int>(next % BASE);
                    carry = next / BASE;
                    j++;
                }
                if (carry > 0) {
                    result.push_back(static_cast<unsigned int>(carry));
                }
            }
        }
                
        for (size_t i = 0; i < z2._digits.size(); ++i) {
            unsigned long long temp = static_cast<unsigned long long>(result[i + 2*m]) + z2._digits[i];
            result[i + 2*m] = static_cast<unsigned int>(temp % BASE);
            if (temp >= BASE) {
                unsigned long long carry = temp / BASE;
                size_t j = i + 2*m + 1;
                while (carry > 0 && j < result.size()) {
                    unsigned long long next = static_cast<unsigned long long>(result[j]) + carry;
                    result[j] = static_cast<unsigned int>(next % BASE);
                    carry = next / BASE;
                    j++;
                }
                if (carry > 0) {
                    result.push_back(static_cast<unsigned int>(carry));
                }
            }
        }
        
        _sign = (_sign == other._sign);
        _digits = std::move(result);
        optimise(_digits);
        return *this;
    }
    else { 
        big_int result(_digits.get_allocator());
        result._digits.resize(_digits.size() + other._digits.size(), 0);

        for (size_t i = 0; i < _digits.size(); ++i)
        {
            unsigned long long carry = 0;
            for (size_t j = 0; j < other._digits.size() || carry; ++j)
            {
                unsigned long long product = result._digits[i + j] + carry;
                if (j < other._digits.size())
                {
                    product += static_cast<unsigned long long>(_digits[i]) * other._digits[j];
                }
                result._digits[i + j] = static_cast<unsigned int>(product % BASE);
                carry = product / BASE;
            }
        }

        _sign = (_sign == other._sign);
        _digits = std::move(result._digits);
        optimise(_digits);
        return *this;
    }
}

big_int& big_int::operator*=(const big_int& other) &
{
    
    multiplication_rule rule = decide_mult(other._digits.size());
    
    
    return multiply_assign(other, rule);
}

big_int& big_int::divide_assign(const big_int& other, division_rule rule) &
{
    
    if (is_zero(other._digits))
    {
        throw std::logic_error("Division by zero");
    }

    if (is_zero(_digits))
    {
        return *this;
    }

    big_int abs_this(*this);
    abs_this._sign = true;
    big_int abs_other(other);
    abs_other._sign = true;

    if (abs_this < abs_other)
    {
        _digits.clear();
        _digits.push_back(0);
        _sign = true;
        return *this;
    }
    
    std::vector<unsigned int, pp_allocator<unsigned int>> quotient(_digits.size() - other._digits.size() + 1, 0, _digits.get_allocator());
    big_int remainder(_digits.get_allocator());
    remainder._digits.clear();
    remainder._digits.push_back(0);
    
    for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; --i)
    {
        remainder._digits.insert(remainder._digits.begin(), _digits[i]);
        optimise(remainder._digits);
    
        if (remainder < abs_other)
        {
            if (i < static_cast<int>(quotient.size()))
                quotient[i] = 0;
            continue;
        }

        unsigned long long left = 0, right = BASE - 1;
        unsigned long long q = 0;
        while (left <= right)
        {
            unsigned long long mid = left + (right - left) / 2;
            big_int temp(abs_other);
            temp *= big_int(static_cast<long long>(mid), _digits.get_allocator());
            if (remainder >= temp)
            {
                q = mid;
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

    
        if (q > 0)
        {
            big_int temp(abs_other);
            temp *= big_int(static_cast<long long>(q), _digits.get_allocator());
            remainder -= temp;
        }

    
        if (i < static_cast<int>(quotient.size()))
            quotient[i] = static_cast<unsigned int>(q);
    }

    _digits = std::move(quotient);
    _sign = (_sign == other._sign);
    optimise(_digits);
    if (is_zero(_digits))
    {
        _sign = true;
    }
    
    return *this;
}


big_int& big_int::operator/=(const big_int& other) &
{
    return divide_assign(other, decide_div(other._digits.size()));
}


big_int& big_int::operator%=(const big_int& other) &
{
    return modulo_assign(other, decide_div(other._digits.size()));
}

big_int big_int::operator+(const big_int& other) const
{
    big_int result(*this);
    result += other;
    return result;
}

big_int big_int::operator-(const big_int& other) const
{
    big_int result(*this);
    result -= other;
    return result;
}

big_int big_int::operator*(const big_int& other) const
{
    big_int result(*this);
    result *= other;
    return result;
}

big_int big_int::operator/(const big_int& other) const
{
    big_int result(*this);
    result /= other;
    return result;
}

big_int big_int::operator%(const big_int& other) const
{
    big_int result(*this);
    result %= other;
    return result;
}

std::strong_ordering big_int::operator<=>(const big_int& other) const noexcept
{

    if (_sign != other._sign)
    {

        return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
    }

    bool is_positive = _sign;
    
    if (_digits.size() != other._digits.size())
    {
        if (is_positive)
        {
            return _digits.size() <=> other._digits.size();
        }
        else
        {

            return other._digits.size() <=> _digits.size();
        }
    }

    for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; --i)
    {
        if (_digits[i] != other._digits[i])
        {
            if (is_positive)
            {
                return _digits[i] <=> other._digits[i];
            }
            else
            {

                return other._digits[i] <=> _digits[i];
            }
        }
    }


    return std::strong_ordering::equal;
}

bool big_int::operator==(const big_int& other) const noexcept
{
    return (*this <=> other) == std::strong_ordering::equal;
}

bool big_int::operator<(const big_int& other) const noexcept
{
    return (*this <=> other) == std::strong_ordering::less;
}

bool big_int::operator>(const big_int& other) const noexcept
{
    return (*this <=> other) == std::strong_ordering::greater;
}

bool big_int::operator<=(const big_int& other) const noexcept
{
    return (*this <=> other) != std::strong_ordering::greater;
}

bool big_int::operator>=(const big_int& other) const noexcept
{
    return (*this <=> other) != std::strong_ordering::less;
}

big_int& big_int::operator<<=(size_t shift) &
{
    if (shift == 0 || is_zero(_digits))
    {
        return *this;
    }

    size_t word_shift = shift / (8 * sizeof(unsigned int));
    size_t bit_shift = shift % (8 * sizeof(unsigned int));

    if (word_shift > 0)
    {
        _digits.insert(_digits.begin(), word_shift, 0);
    }

    if (bit_shift > 0)
    {
        unsigned long long carry = 0;
        for (size_t i = 0; i < _digits.size(); ++i)
        {
            unsigned long long value = (static_cast<unsigned long long>(_digits[i]) << bit_shift) | carry;
            _digits[i] = static_cast<unsigned int>(value % BASE);
            carry = value / BASE;
        }
        if (carry > 0)
        {
            _digits.push_back(static_cast<unsigned int>(carry));
        }
    }

    optimise(_digits);
    return *this;
}

big_int& big_int::operator>>=(size_t shift) &
{
    if (shift == 0 || is_zero(_digits))
    {
        return *this;
    }

    size_t word_shift = shift / (8 * sizeof(unsigned int));
    size_t bit_shift = shift % (8 * sizeof(unsigned int));

    if (word_shift >= _digits.size())
    {
        _digits.clear();
        _digits.push_back(0);
        _sign = true;
        return *this;
    }
    if (word_shift > 0)
    {
        _digits.erase(_digits.begin(), _digits.begin() + word_shift);
    }

    if (bit_shift > 0)
    {
        unsigned long long carry = 0;
        for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; --i)
        {
            unsigned long long value = (carry << (8 * sizeof(unsigned int))) | _digits[i];
            _digits[i] = static_cast<unsigned int>(value >> bit_shift);
            carry = (value & ((1ULL << bit_shift) - 1));
        }
    }

    optimise(_digits);
    if (is_zero(_digits))
    {
        _sign = true;
    }
    return *this;
}

big_int big_int::operator<<(size_t shift) const
{
    big_int result(*this);
    result <<= shift;
    return result;
}

big_int big_int::operator>>(size_t shift) const
{
    big_int result(*this);
    result >>= shift;
    return result;
}

big_int big_int::operator~() const
{
    big_int result(*this);
    for (auto& digit : result._digits)
    {
        digit = ~digit;
    }
    optimise(result._digits);
    return result;
}

big_int& big_int::operator&=(const big_int& other) &
{
    size_t max_size = std::max(_digits.size(), other._digits.size());
    _digits.resize(max_size, 0);
    
    for (size_t i = 0; i < max_size; ++i)
    {
        if (i < other._digits.size()) {
            _digits[i] &= other._digits[i];
        } else {
            _digits[i] = 0; 
        }
    }
    
    optimise(_digits);
    return *this;
}

big_int& big_int::operator|=(const big_int& other) &
{
    size_t max_size = std::max(_digits.size(), other._digits.size());
    _digits.resize(max_size, 0);
    
    for (size_t i = 0; i < other._digits.size(); ++i)
    {
        _digits[i] |= other._digits[i];
    }
    
    optimise(_digits);
    return *this;
}

big_int& big_int::operator^=(const big_int& other) &
{
    size_t max_size = std::max(_digits.size(), other._digits.size());
    _digits.resize(max_size, 0);
    
    for (size_t i = 0; i < other._digits.size(); ++i)
    {
        _digits[i] ^= other._digits[i];
    }
    
    optimise(_digits);
    return *this;
}

big_int big_int::operator&(const big_int& other) const
{
    big_int result(*this);
    result &= other;
    return result;
}

big_int big_int::operator|(const big_int& other) const
{
    big_int result(*this);
    result |= other;
    return result;
}

big_int big_int::operator^(const big_int& other) const
{
    big_int result(*this);
    result ^= other;
    return result;
}

std::ostream& operator<<(std::ostream& stream, const big_int& value)
{
    stream << value.to_string();
    return stream;
}

std::istream& operator>>(std::istream& stream, big_int& value)
{
    std::string num;
    stream >> num;
    value = big_int(num, 10, value._digits.get_allocator());
    return stream;
}

std::string big_int::to_string() const
{
    if (is_zero(_digits))
    {
        return "0";
    }

    std::string result;
    big_int temp(*this);
    temp._sign = true;

    while (temp)
    {
        big_int remainder = temp % 10;
        result += static_cast<char>('0' + remainder._digits[0]);
        temp /= 10;
    }

    if (!_sign)
    {
        result += '-';
    }

    std::reverse(result.begin(), result.end());
    return result;
}

big_int operator""_bi(unsigned long long n)
{
    return big_int(static_cast<long long>(n));
}

big_int& big_int::modulo_assign(const big_int& other, division_rule rule) &
{

    if (is_zero(other._digits))
    {
        throw std::logic_error("Division by zero");
    }

    if (is_zero(_digits))
    {
        return *this;
    }

    big_int abs_this(*this);
    abs_this._sign = true;
    big_int abs_other(other);
    abs_other._sign = true;

    if (abs_this < abs_other)
    {

        _sign = true;
        return *this;
    }

    big_int remainder(_digits.get_allocator());
    remainder._digits.clear();
    remainder._digits.push_back(0);

    for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; --i)
    {
        remainder._digits.insert(remainder._digits.begin(), _digits[i]);
        optimise(remainder._digits);

        if (remainder < abs_other)
        {
            continue;
        }

        unsigned long long left = 0, right = BASE - 1;
        unsigned long long q = 0;
        while (left <= right)
        {
            unsigned long long mid = left + (right - left) / 2;
            big_int temp(abs_other);
            temp *= big_int(static_cast<long long>(mid), _digits.get_allocator());
            if (remainder >= temp)
            {
                q = mid;
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        if (q > 0)
        {
            big_int temp(abs_other);
            temp *= big_int(static_cast<long long>(q), _digits.get_allocator());
            remainder -= temp;
        }
    }
    _digits = std::move(remainder._digits);
    if (is_zero(_digits))
    {
        _sign = true;
    }
    
    return *this;
}