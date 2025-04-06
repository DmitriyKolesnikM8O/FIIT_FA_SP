//
// Created by Des Caldnd on 5/27/2024.
//

#ifndef MP_OS_BIG_INT_H
#define MP_OS_BIG_INT_H

#include <vector>
#include <utility>
#include <iostream>
#include <concepts>
#include <pp_allocator.h>
#include <not_implemented.h>

namespace __detail
{
    constexpr unsigned int generate_half_mask()
    {
        unsigned int res = 0;

        for(size_t i = 0; i < sizeof(unsigned int) * 4; ++i)
        {
            res |= (1u << i);
        }

        return res;
    }

    constexpr size_t nearest_greater_power_of_2(size_t size) noexcept
    {
        int ones_counter = 0, index = -1;

        constexpr const size_t o = 1;

        for (int i = sizeof(size_t) * 8 - 1; i >= 0; --i)
        {
            if (size & (o << i))
            {
                if (ones_counter == 0)
                    index = i;
                ++ones_counter;
            }
        }

        return ones_counter <= 1 ? (1u << index) : (1u << (index + 1));
    }
}

class big_int
{
    // Call optimise after every operation!!!
    bool _sign; // 1 +  0 -
    std::vector<unsigned int, pp_allocator<unsigned int>> _digits;

public:

    enum class multiplication_rule
    {
        trivial,
        Karatsuba,
        SchonhageStrassen
    };

    enum class division_rule
    {
        trivial,
        Newton,
        BurnikelZiegler
    };

private:

    /** Decides type of mult/div that depends on size of lhs and rhs
     */
    multiplication_rule decide_mult(size_t rhs) const noexcept;
    division_rule decide_div(size_t rhs) const noexcept;

public:

    using value_type = unsigned int;

    template<class alloc>
    explicit big_int(const std::vector<unsigned int, alloc> &digits, bool sign = true, pp_allocator<unsigned int> allocator = pp_allocator<unsigned int>());

    explicit big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign = true);

    explicit big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign = true) noexcept;

    explicit big_int(const std::string& num, unsigned int radix = 10, pp_allocator<unsigned int> = pp_allocator<unsigned int>());

    template<std::integral Num>
    big_int(Num d, pp_allocator<unsigned int> = pp_allocator<unsigned int>());

    big_int(pp_allocator<unsigned int> = pp_allocator<unsigned int>());

    explicit operator bool() const noexcept; //false if 0 , else true

    big_int& operator++() &;
    big_int operator++(int);

    big_int& operator--() &;
    big_int operator--(int);

    big_int& operator+=(const big_int& other) &;

    /** Shift will be needed for multiplication implementation
     *  @example Shift = 0: 111 + 222 = 333
     *  @example Shift = 1: 111 + 222 = 2331
     */
    big_int& plus_assign(const big_int& other, size_t shift = 0) &;

    big_int& operator-=(const big_int& other) &;

    big_int& minus_assign(const big_int& other, size_t shift = 0) &;

    /** Delegates to multiply_assign and calls decide_mult
     */
    big_int& operator*=(const big_int& other) &;

    big_int& multiply_assign(const big_int& other, multiplication_rule rule = multiplication_rule::trivial) &;

    big_int& operator/=(const big_int& other) &;

    big_int& divide_assign(const big_int& other, division_rule rule = division_rule::trivial) &;

    big_int& operator%=(const big_int& other) &;

    big_int& modulo_assign(const big_int& other, division_rule rule = division_rule::trivial) &;

    big_int operator+(const big_int& other) const;
    big_int operator-(const big_int& other) const;
    big_int operator*(const big_int& other) const;
    big_int operator/(const big_int& other) const;
    big_int operator%(const big_int& other) const;

    std::strong_ordering operator<=>(const big_int& other) const noexcept;

    bool operator==(const big_int& other) const noexcept;
    bool operator<(const big_int& other) const noexcept;
    bool operator>(const big_int& other) const noexcept;
    bool operator<=(const big_int& other) const noexcept;
    bool operator>=(const big_int& other) const noexcept;

    big_int& operator<<=(size_t shift) &;

    big_int& operator>>=(size_t shift) &;

    big_int operator<<(size_t shift) const;
    big_int operator>>(size_t shift) const;

    big_int operator~() const;

    big_int& operator&=(const big_int& other) &;

    big_int& operator|=(const big_int& other) &;

    big_int& operator^=(const big_int& other) &;

    big_int operator&(const big_int& other) const;
    big_int operator|(const big_int& other) const;
    big_int operator^(const big_int& other) const;

    friend std::ostream &operator<<(std::ostream &stream, big_int const &value);

    friend std::istream &operator>>(std::istream &stream, big_int &value);

    std::string to_string() const;
};

// Реализация шаблонного конструктора из вектора
template<class alloc>
big_int::big_int(const std::vector<unsigned int, alloc> &digits, bool sign, pp_allocator<unsigned int> allocator)
    : _sign(sign), _digits(digits.begin(), digits.end(), allocator)
{
    if (_digits.empty())
    {
        _digits.push_back(0);
    }
    // Оптимизация: удаляем ведущие нули
    while (_digits.size() > 1 && _digits.back() == 0)
    {
        _digits.pop_back();
    }
}

// Реализация шаблонного конструктора из числа
template<std::integral Num>
big_int::big_int(Num d, pp_allocator<unsigned int> allocator)
    : _sign(d >= 0), _digits(allocator)
{
    unsigned long long abs_d = static_cast<unsigned long long>(d < 0 ? -d : d);
    _digits.clear();
    if (abs_d == 0)
    {
        _digits.push_back(0);
    }
    else
    {
        constexpr unsigned long long BASE = 1ULL << (8 * sizeof(unsigned int));
        while (abs_d > 0)
        {
            _digits.push_back(static_cast<unsigned int>(abs_d % BASE));
            abs_d /= BASE;
        }
    }
    // Оптимизация: удаляем ведущие нули
    while (_digits.size() > 1 && _digits.back() == 0)
    {
        _digits.pop_back();
    }
}

big_int operator""_bi(unsigned long long n);

#endif //MP_OS_BIG_INT_H