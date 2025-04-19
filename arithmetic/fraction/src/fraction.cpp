#include "../include/fraction.h"
#include <cmath>
#include <numeric>
#include <sstream>

// Helper function to calculate greatest common divisor (GCD)
big_int gcd(big_int a, big_int b) {
    // Ensure positive values
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    
    // Euclidean algorithm
    while (b != 0) {
        const big_int t = b;
        b = a % b;
        a = t;
    }
    
    return a;
}

void fraction::optimise()
{
    // If denominator is zero, throw an exception
    if (_denominator == 0) {
        throw std::invalid_argument("Denominator cannot be zero");
    }
    
    // If numerator is zero, set denominator to 1
    if (_numerator == 0) {
        _denominator = 1;
        return;
    }
    
    // Get the greatest common divisor
    const big_int divisor = gcd(_numerator, _denominator);
    
    // Divide both numerator and denominator by the GCD
    _numerator /= divisor;
    _denominator /= divisor;
    
    // Ensure that the sign is in the numerator (denominator always positive)
    if (_denominator < 0) {
        _numerator = -_numerator;
        _denominator = -_denominator;
    }
}

template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f &&numerator, s &&denominator)
    : _numerator(std::forward<f>(numerator)), 
      _denominator(std::forward<s>(denominator)) {
    optimise();
}

fraction::fraction(const pp_allocator<big_int::value_type> allocator)
    : _numerator(0, allocator), _denominator(1, allocator) {
}

fraction &fraction::operator+=(fraction const &other) &
{
    // a/b + c/d = (a*d + b*c)/(b*d)
    _numerator = _numerator * other._denominator + _denominator * other._numerator;
    _denominator = _denominator * other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator+(fraction const &other) const
{
    fraction result = *this;
    result += other;
    return result;
}

fraction &fraction::operator-=(fraction const &other) &
{
    // a/b - c/d = (a*d - b*c)/(b*d)
    _numerator = _numerator * other._denominator - _denominator * other._numerator;
    _denominator = _denominator * other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const
{
    fraction result = *this;
    result -= other;
    return result;
}

fraction fraction::operator-() const {
    return *this * fraction(-1, 1);
}

fraction &fraction::operator*=(fraction const &other) &
{
    // a/b * c/d = (a*c)/(b*d)
    _numerator *= other._numerator;
    _denominator *= other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator*(fraction const &other) const
{
    fraction result = *this;
    result *= other;
    return result;
}

fraction &fraction::operator/=(fraction const &other) &
{
    // a/b / c/d = (a*d)/(b*c)
    if (other._numerator == 0) {
        throw std::invalid_argument("Division by zero");
    }
    
    _numerator *= other._denominator;
    _denominator *= other._numerator;
    optimise();
    return *this;
}

fraction fraction::operator/(fraction const &other) const
{
    fraction result = *this;
    result /= other;
    return result;
}

bool fraction::operator==(fraction const &other) const noexcept
{
    // After optimise(), if two fractions are equal, they should have the same numerator and denominator
    return _numerator == other._numerator && _denominator == other._denominator;
}

std::partial_ordering fraction::operator<=>(const fraction& other) const noexcept
{
    // Handle potential overflow by cross-multiplying
    // a/b <=> c/d equivalent to a*d <=> b*c when b,d > 0 (which is guaranteed by optimise())
    
    // Check for zero denominators to avoid undefined behavior
    if (_denominator == 0 || other._denominator == 0) {
        return std::partial_ordering::unordered;
    }
    
    // Compare a*d with b*c
    const big_int lhs = _numerator * other._denominator;
    const big_int rhs = _denominator * other._numerator;
    
    if (lhs < rhs) return std::partial_ordering::less;
    if (lhs > rhs) return std::partial_ordering::greater;
    return std::partial_ordering::equivalent;
}

std::ostream &operator<<(std::ostream &stream, fraction const &obj)
{
    stream << obj.to_string();
    return stream;
}

std::istream &operator>>(std::istream &stream, fraction &obj)
{
    // Format expected: "numerator/denominator"
    std::string input;
    stream >> input;

    if (const size_t delimiter_pos = input.find('/'); delimiter_pos != std::string::npos) {
        const std::string numerator_str = input.substr(0, delimiter_pos);
        const std::string denominator_str = input.substr(delimiter_pos + 1);
        
        big_int numerator(numerator_str, 10);
        big_int denominator(denominator_str, 10);
        
        obj = fraction(numerator, denominator);
    } else {
        // If no delimiter, interpret as numerator with denominator 1
        big_int numerator(input, 10);
        obj = fraction(numerator, big_int(1));
    }
    
    return stream;
}

std::string fraction::to_string() const
{
    std::stringstream ss;
    ss << _numerator << "/" << _denominator;
    return ss.str();
}

fraction fraction::sin(fraction const &epsilon) const
{
    const fraction x = *this;
    fraction result(0, 1);
    fraction term = x;
    fraction factorial(1, 1);
    int sign = 1;
    int n = 0;
    
    while (term > epsilon || term < -epsilon) {
        result += fraction(sign, 1) * term;
        
        // Next term calculation
        sign = -sign;
        n += 2;
        term *= x * x;
        factorial._numerator *= n * (n - 1);
        term /= factorial;
    }
    
    return result;
}

fraction fraction::cos(fraction const &epsilon) const
{
    const fraction x = *this;
    fraction result(1, 1);
    fraction term(1, 1);
    fraction factorial(1, 1);
    int sign = 1;
    int n = 0;
    
    while (term > epsilon || term < -epsilon) {
        result += fraction(sign, 1) * term;
        
        // Next term calculation
        sign = -sign;
        n += 2;
        term *= x * x;
        factorial._numerator *= n * (n - 1);;
        term /= factorial;
        
        // Skip the first iteration as we've already added 1
        if (n == 2) result -= term;
    }
    
    return result;
}

fraction fraction::tg(fraction const &epsilon) const
{
    const fraction cosine = this->cos(epsilon);
    if (cosine._numerator == 0) {
        throw std::domain_error("Tangent undefined at this point");
    }
    return this->sin(epsilon) / cosine;
}

fraction fraction::ctg(fraction const &epsilon) const
{
    const fraction sine = this->sin(epsilon);
    if (sine._numerator == 0) {
        throw std::domain_error("Cotangent undefined at this point");
    }
    return this->cos(epsilon) / sine;
}

fraction fraction::sec(fraction const &epsilon) const
{
    const fraction cosine = this->cos(epsilon);
    if (cosine._numerator == 0) {
        throw std::domain_error("Secant undefined at this point");
    }
    return fraction(1, 1) / cosine;
}

fraction fraction::cosec(fraction const &epsilon) const
{
    const fraction sine = this->sin(epsilon);
    if (sine._numerator == 0) {
        throw std::domain_error("Cosecant undefined at this point");
    }
    return fraction(1, 1) / sine;
}

fraction fraction::pow(size_t degree) const
{
    if (degree == 0) {
        return fraction(1, 1);
    }
    
    // For negative numbers with even exponents, result is positive
    // For negative numbers with odd exponents, result is negative
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

fraction fraction::root(size_t degree, fraction const &epsilon) const
{
    if (degree == 0) {
        throw std::invalid_argument("Degree cannot be zero");
    }
    
    if (degree == 1) {
        return *this;
    }
    
    // For even degree, negative numbers don't have a real root
    if (_numerator < 0 && degree % 2 == 0) {
        throw std::domain_error("Even root of negative number is not a real number");
    }
    
    // Starting guess: for positive x, sqrt(x) ~ x/2, cube root ~ x/3, etc.
    fraction x = *this;
    if (x._numerator < 0) x = -x; // Work with absolute value for odd roots
    
    fraction guess = *this / fraction(degree, 1);
    fraction prev_guess;
    
    do {
        prev_guess = guess;
        // Newton's formula: x_{n+1} = ((n-1)*x_n + a/x_n^(n-1))/n
        fraction power = guess.pow(degree - 1);
        guess = (fraction(degree - 1, 1) * guess + *this / power) / fraction(degree, 1);
    } while ((guess - prev_guess > epsilon) || (prev_guess - guess > epsilon));
    
    // If original number was negative and degree is odd, result should be negative
    if (_numerator < 0 && degree % 2 == 1) {
        guess = -guess;
    }
    
    return guess;
}

fraction fraction::log2(fraction const &epsilon) const
{
    if (_numerator <= 0 || _denominator <= 0) {
        throw std::domain_error("Logarithm of non-positive number is undefined");
    }
    
    // log2(x) = ln(x) / ln(2)
    const fraction result = this->ln(epsilon);
    fraction ln2(0, 1); // Natural log of 2
    
    // Calculate ln(2) using the series ln(1+y) = y - y^2/2 + y^3/3 - ... with y = 1
    const fraction y(1, 1);
    fraction term = y;
    int n = 1;
    
    while (term > epsilon || term < -epsilon) {
        ln2 += fraction(term._numerator * ((n % 2 == 1) ? 1 : -1), term._denominator * n);
        ++n;
        term *= y;
    }
    
    return result / ln2;
}

fraction fraction::ln(fraction const &epsilon) const
{
    if (_numerator <= 0 || _denominator <= 0) {
        throw std::domain_error("Natural logarithm of non-positive number is undefined");
    }
    
    // For x near 1, ln(1+y) = y - y^2/2 + y^3/3 - ...
    fraction x = *this;
    fraction y = x - fraction(1, 1); // y = x - 1
    
    if (y > fraction(1, 2) || y < fraction(-1, 3)) {
        // Use ln(a*b) = ln(a) + ln(b) to reduce the problem
        // We want to find a value z such that x/z is near 1
        // Using 2 as a factor to reduce x
        if (x > fraction(1, 1)) {
            fraction half = x / fraction(2, 1);
            return fraction(1, 1) + half.ln(epsilon);
        } else {
            // For x < 1, use ln(1/x) = -ln(x)
            fraction recip = fraction(1, 1) / x;
            return -recip.ln(epsilon);
        }
    }
    
    // Calculate using Taylor series
    fraction result(0, 1);
    fraction term = y;
    int n = 1;
    
    while (term > epsilon || term < -epsilon) {
        result += fraction(term._numerator * ((n % 2 == 1) ? 1 : -1), term._denominator * n);
        ++n;
        term *= y;
    }
    
    return result;
}

fraction fraction::lg(fraction const &epsilon) const
{
    if (_numerator <= 0 || _denominator <= 0) {
        throw std::domain_error("Base-10 logarithm of non-positive number is undefined");
    }
    
    // log10(x) = ln(x) / ln(10)
    const fraction result = this->ln(epsilon);
    fraction ln10(0, 1); // Natural log of 10
    
    // Calculate ln(10)
    const fraction y(9, 1); // 10 - 1 = 9
    fraction term = y;
    int n = 1;
    
    while (term > epsilon || term < -epsilon) {
        ln10 += fraction(term._numerator * ((n % 2 == 1) ? 1 : -1), term._denominator * n);
        ++n;
        term *= y / fraction(n, 1);
    }
    
    return result / ln10;
}