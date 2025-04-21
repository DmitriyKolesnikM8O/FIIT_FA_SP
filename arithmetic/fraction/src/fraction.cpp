#include "../include/fraction.h"
#include <cmath>
#include <numeric>
#include <sstream>
#include <regex>

// Helper function to calculate greatest common divisor (GCD)
big_int gcd(big_int a, big_int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        big_int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

void fraction::optimise() {
    if (_denominator == 0) {
        throw std::invalid_argument("Denominator cannot be zero");
    }
    if (_numerator == 0) {
        _denominator = 1;
        return;
    }
    big_int divisor = gcd(_numerator, _denominator);
    _numerator /= divisor;
    _denominator /= divisor;
    if (_denominator < 0) {
        _numerator = -_numerator;
        _denominator = -_denominator;
    }
}

template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f &&numerator, s &&denominator)
    : _numerator(std::forward<f>(numerator)),
      _denominator(std::forward<s>(denominator)) {
    if (_denominator == 0) {
        throw std::invalid_argument("Denominator cannot be zero");
    }
    optimise();
}

fraction::fraction(const pp_allocator<big_int::value_type> allocator)
    : _numerator(0, allocator), _denominator(1, allocator) {
}

fraction &fraction::operator+=(fraction const &other) & {
    _numerator = _numerator * other._denominator + _denominator * other._numerator;
    _denominator = _denominator * other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator+(fraction const &other) const {
    fraction result = *this;
    result += other;
    return result;
}

fraction &fraction::operator-=(fraction const &other) & {
    _numerator = _numerator * other._denominator - _denominator * other._numerator;
    _denominator = _denominator * other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const {
    fraction result = *this;
    result -= other;
    return result;
}

fraction fraction::operator-() const {
    fraction result(*this);
    result._numerator = -result._numerator;
    result.optimise();
    return result;
}

fraction &fraction::operator*=(fraction const &other) & {
    _numerator *= other._numerator;
    _denominator *= other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator*(fraction const &other) const {
    fraction result = *this;
    result *= other;
    return result;
}

fraction &fraction::operator/=(fraction const &other) & {
    if (other._numerator == 0) {
        throw std::invalid_argument("Division by zero");
    }
    _numerator *= other._denominator;
    _denominator *= other._numerator;
    optimise();
    return *this;
}

fraction fraction::operator/(fraction const &other) const {
    fraction result = *this;
    result /= other;
    return result;
}

bool fraction::operator==(fraction const &other) const noexcept {
    return _numerator == other._numerator && _denominator == other._denominator;
}

std::partial_ordering fraction::operator<=>(const fraction& other) const noexcept {
    big_int lhs = _numerator * other._denominator;
    big_int rhs = _denominator * other._numerator;
    if (lhs < rhs) return std::partial_ordering::less;
    if (lhs > rhs) return std::partial_ordering::greater;
    return std::partial_ordering::equivalent;
}

std::ostream &operator<<(std::ostream &stream, fraction const &obj) {
    stream << obj.to_string();
    return stream;
}

std::istream &operator>>(std::istream &stream, fraction &obj) {
    std::string input;
    stream >> input;
    std::regex fraction_regex(R"(^([-+]?[0-9]+)(?:/([-+]?[0-9]+))?$)");
    std::smatch match;
    if (std::regex_match(input, match, fraction_regex)) {
        big_int numerator(match[1].str(), 10);
        big_int denominator(1);
        if (match[2].matched) {
            denominator = big_int(match[2].str(), 10);
        }
        obj = fraction(numerator, denominator);
    } else {
        throw std::invalid_argument("Invalid fraction format");
    }
    return stream;
}

std::string fraction::to_string() const {
    std::stringstream ss;
    ss << _numerator << "/" << _denominator;
    return ss.str();
}

fraction fraction::sin(fraction const &epsilon) const {
    fraction x = *this;
    fraction result(0, 1);
    fraction term = x;
    big_int factorial = 1;
    int n = 1;
    while (term > epsilon || term < -epsilon) {
        result += term;
        n += 2;
        factorial *= n * (n - 1);
        term *= x * x;
        term /= fraction(factorial, 1);
        term = -term;
    }
    return result;
}

fraction fraction::cos(fraction const &epsilon) const {
    fraction x = *this;
    fraction result(1, 1);
    fraction term(1, 1);
    big_int factorial = 1;
    int n = 0;
    while (term > epsilon || term < -epsilon) {
        if (n > 0) {
            result += term;
        }
        n += 2;
        factorial *= n * (n - 1);
        term *= x * x;
        term /= fraction(factorial, 1);
        term = -term;
    }
    return result;
}

fraction fraction::tg(fraction const &epsilon) const {
    fraction cosine = this->cos(epsilon);
    if (cosine._numerator == 0) {
        throw std::domain_error("Tangent undefined");
    }
    return this->sin(epsilon) / cosine;
}

fraction fraction::ctg(fraction const &epsilon) const {
    fraction sine = this->sin(epsilon);
    if (sine._numerator == 0) {
        throw std::domain_error("Cotangent undefined");
    }
    return this->cos(epsilon) / sine;
}

fraction fraction::sec(fraction const &epsilon) const {
    fraction cosine = this->cos(epsilon);
    if (cosine._numerator == 0) {
        throw std::domain_error("Secant undefined");
    }
    return fraction(1, 1) / cosine;
}

fraction fraction::cosec(fraction const &epsilon) const {
    fraction sine = this->sin(epsilon);
    if (sine._numerator == 0) {
        throw std::domain_error("Cosecant undefined");
    }
    return fraction(1, 1) / sine;
}

fraction fraction::arctg(fraction const &epsilon) const {
    if (_numerator < 0) {
        return -(-*this).arctg(epsilon);
    }
    if (*this > fraction(1, 1)) {
        return fraction(1, 2) - (fraction(1, 1) / *this).arctg(epsilon);
    }
    fraction result(0, 1);
    fraction term = *this;
    int n = 1;
    while (term > epsilon || term < -epsilon) {
        result += fraction((n % 2 == 0 ? -1 : 1), n) * term;
        n += 2;
        term *= *this * *this;
    }
    return result;
}

fraction fraction::pow(size_t degree) const {
    if (degree == 0) {
        return fraction(1, 1);
    }
    fraction base = *this;
    fraction result(1, 1);
    while (degree > 0) {
        if (degree & 1) {
            result *= base;
        }
        base *= base;
        degree >>= 1;
    }
    return result;
}

fraction fraction::root(size_t degree, fraction const &epsilon) const {
    if (degree == 0) {
        throw std::invalid_argument("Degree cannot be zero");
    }
    if (degree == 1) {
        return *this;
    }
    if (_numerator < 0 && degree % 2 == 0) {
        throw std::domain_error("Even root of negative number is not real");
    }
    fraction x = *this;
    if (x._numerator < 0) x = -x;
    fraction guess = *this / fraction(degree, 1);
    fraction prev_guess;
    do {
        prev_guess = guess;
        fraction power = guess.pow(degree - 1);
        if (power._numerator == 0) {
            throw std::runtime_error("Division by zero in root calculation");
        }
        guess = (fraction(degree - 1, 1) * guess + *this / power) / fraction(degree, 1);
    } while ((guess - prev_guess > epsilon) || (prev_guess - guess > epsilon));
    if (_numerator < 0 && degree % 2 == 1) {
        guess = -guess;
    }
    return guess;
}

fraction fraction::log2(fraction const &epsilon) const {
    if (_numerator <= 0 || _denominator <= 0) {
        throw std::domain_error("Logarithm of non-positive number is undefined");
    }
    fraction ln2 = fraction(2, 1).ln(epsilon);
    return this->ln(epsilon) / ln2;
}

fraction fraction::ln(fraction const &epsilon) const {
    if (_numerator <= 0 || _denominator <= 0) {
        throw std::domain_error("Natural logarithm of non-positive number is undefined");
    }
    fraction x = *this;
    if (x > fraction(2, 1)) {
        return fraction(1, 1).ln(epsilon) + (x / fraction(2, 1)).ln(epsilon);
    }
    if (x < fraction(1, 2)) {
        return -(fraction(1, 1) / x).ln(epsilon);
    }
    fraction y = x - fraction(1, 1);
    fraction result(0, 1);
    fraction term = y;
    int n = 1;
    while (term > epsilon || term < -epsilon) {
        result += fraction((n % 2 == 0 ? -1 : 1), n) * term;
        n++;
        term *= y;
    }
    return result;
}

fraction fraction::lg(fraction const &epsilon) const {
    if (_numerator <= 0 || _denominator <= 0) {
        throw std::domain_error("Base-10 logarithm of non-positive number is undefined");
    }
    fraction ln10 = fraction(10, 1).ln(epsilon);
    return this->ln(epsilon) / ln10;
}
