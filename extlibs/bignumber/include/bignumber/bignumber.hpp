/**
 * BigNumber - a BigInt implementation in C++.
 * This file is licensed under the MIT license.
 * Do not remove this comment.
 * 2018 @ pr0crustes
 */
#ifndef BIGNUMBER_H
#define BIGNUMBER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Use an exclusive Debug Flag.
 * When defined, the lib will run slower,
 * but will performe multiple checks to make everything is fine.
 */
//#define BIG_NUMBER_DEBUG   // Use a exclusive debug flag

/**
 * @brief The BigNumber class represents a limitless number.
 * Can hold and operate values way larger than 2 to 64.
 */
class BigNumber
{

public:
    /*
     * Constructors.
     * BigNumber has 0 as default value.
     * Can be instantiated with the value of a string or long long.
     */
    BigNumber(std::string string);
    BigNumber(const BigNumber &) = default;
    BigNumber();
    BigNumber(long long value);

    /**
     * @brief from_binary instantiates a BigNumber from a binary string representation.
     * @param binary a binary like string. No checking is done to see if it is valid, any char other than 0 and 1 can lead to undefined behavior.
     * @param is_signed if true, as default, the first bit will be interpreted as the signal, like any signed integers.
     * @return a BigNumber with the value of the binary string.
     */
    static BigNumber from_binary(std::string binary, bool is_signed = true) noexcept;

    /**
     * @brief static method that generate random BigNumbers.
     * THIS IS A PSEUDO-RANDOM FUNCTION. DO NOT RELY IN IT BEING COMPLETELY RANDOM.
     * @param size the desired BigNumber size.
     * @return a random BigNumber with the desired number of digits.
     */
    static BigNumber random_big_number(int length);

    /**
     * @brief random_big_number_in_range generate a random BigNumber in desired range, excluding higher.
     * This method may be a little slow compared to the others, had'nt any better idea.
     * Does not work with negative numbers.
     * THIS IS A PSEUDO-RANDOM FUNCTION. DO NOT RELY IN IT BEING COMPLETELY RANDOM.
     * @param lower the lower bound, inclusive.
     * @param higher the upper bound, exclusive.
     * @return a random BigNumber in range.
     */
    static BigNumber random_big_number_in_range(const BigNumber &min_bound, const BigNumber &max_bound);

    /*
     * Copy constructor.
     */
    /**
     * @brief operator = copy constructor. Copy the vector and the sign.
     * @param number
     * @return
     */
    BigNumber &operator=(const BigNumber &number) noexcept;

    /*
     * Unary operators.
     */
    /**
     * @brief operator + This does NOT return the absolute value.
     * @return the BigNumber itself.
     */
    BigNumber operator+() const noexcept;

    /**
     * @brief operator - the same as multiplying by -1.
     * @return returns the bignumber with reverted signal.
     */
    BigNumber operator-() const noexcept;

    /*
     * Binary operators.
     */
    /**
     * @brief operator + plus operator. Add one BigNumber to this.
     * @param number the number to be added.
     * @return the sum of both BigNumbers.
     */
    BigNumber operator+(const BigNumber &number) const noexcept;

    /**
     * @brief operator - minus operator. Subtracts one BigNumber from this.
     * @param number the BigNumber to be subtracted.
     * @return the result of the subtraction
     */
    BigNumber operator-(const BigNumber &number) const noexcept;

    /**
     * @brief operator * the multiplier operator.
     * Uses Karatsuba Algorithm, see https://en.wikipedia.org/wiki/Karatsuba_algorithm for info.
     * @param number the number to be multiplied by.
     * @return the result of the multiplication.
     */
    BigNumber operator*(const BigNumber &number) const noexcept;

    /**
     * @brief operator / divide one number by other.
     * Keep in mind that the return is a BigNumber, so 10 / 4 is 2, not 2.5.
     * If you want the reaminder of a division, see the `%` operator.
     * @param number number to divide by.
     * @return result of division.
     * Throws in case of division by 0.
     */
    BigNumber operator/(const BigNumber &number) const;

    /**
     * @brief operator % module operator. This function does not work with negative numbers.
     * @param number the divisor number.
     * @return the result of the module operator.
     * Throws in case of module by 0.
     */
    BigNumber operator%(const BigNumber &number) const;

    /**
     * @brief pow power method. Solves with Exponentiation by Squaring.
     * Throws an exception in case of 0 to the power of 0 and in case of any number to a negative one.
     * This function is slow with big numbers. If you want to apply a mod after, use mod_pow, since it is faster.
     * See https://en.wikipedia.org/wiki/Exponentiation_by_squaring
     * @param number the desired power.
     * @return THIS to the power of NUMBER.
     */
    BigNumber pow(BigNumber number) const;

    /**
     * @brief mod_pow fast way of doing apower operation followed by module.
     * See https://en.wikipedia.org/wiki/Modular_exponentiation#Memory-efficient_method
     * @param power the power wanted.
     * @param mod the module wanted.
     * @return this to the power power module mod.
     */
    BigNumber mod_pow(const BigNumber &power, const BigNumber &mod) const;

    /*
     * Assigment operators.
     */
    BigNumber &operator+=(const BigNumber &number) noexcept;
    BigNumber &operator-=(const BigNumber &number) noexcept;
    BigNumber &operator*=(const BigNumber &number) noexcept;
    BigNumber &operator/=(const BigNumber &number);
    BigNumber &operator%=(const BigNumber &number);

    /*
     * Increment / Decrement operators.
     */
    BigNumber &operator++() noexcept;
    BigNumber &operator--() noexcept;
    BigNumber operator++(int) noexcept;
    BigNumber operator--(int) noexcept;

    /*
     * Relational operators
     */
    bool operator<(const BigNumber &number) const noexcept;
    bool operator>(const BigNumber &number) const noexcept;
    bool operator<=(const BigNumber &number) const noexcept;
    bool operator>=(const BigNumber &number) const noexcept;
    bool operator==(const BigNumber &number) const noexcept;
    bool operator!=(const BigNumber &number) const noexcept;

    /*
     * >> and << operators. Just for convenience.
     */
    friend std::ostream &operator<<(std::ostream &stream, BigNumber const &number)
    {
        stream << number.as_string();
        return stream;
    }

    friend std::istream &operator>>(std::istream &istream, BigNumber &number)
    {
        std::string in;
        istream >> in;
        number = BigNumber(in);
        return istream;
    }

    /**
     * @brief as_string methot that creates a string representation of a BigNumber.
     * @return a string representing the BigNumber.
     */
    std::string as_string() const noexcept;

    /**
     * @brief absolute_value gets the absolute value of a BigNumber,
     * @return a BigNumber copy, but positive, the absolute value.
     */
    BigNumber absolute_value() const noexcept;

    /**
     * @brief times10 method that multiplies the number by 10, n times. Made to be fast.
     * @param times how many times it shoud be multiplied by 10. Default is 1.
     * @return the number times 10 n times.
     */
    BigNumber times10(int times = 1) const noexcept;

    /**
     * @brief divide10 method that divides the number by 10, n times. Made to be fast.
     * @param times how many times it shoud be divided by 10. Default is 1.
     * @return the number divided by 10 n times.
     */
    BigNumber divide10(int times = 1) const noexcept;

    /**
     * @brief as_binary returns a std::string representation of the BigNumber as binary.
     * The length of the string will always be the smallest necessary to fit the representation, plus the signed bit.
     * The representation is signed-like, so the first char is 1 if the number is negative, 0 if positive.
     * @return a std::string representation of the BigNumber as binary.
     */
    std::string as_binary() const noexcept;

    /**
     * @brief split_at splits a BigNumber at a desired position.
     * @param split_pos the position to be split at.
     * Keep in mind that this position is from right to left, so spliting the number 123456 at pos 1 will result in 12345 and 6.
     * If the param is not in range [0, this.length], it will be capped to it.
     * @return a pair of the result BigNumbers.
     */
    std::pair<BigNumber, BigNumber> split_at(long long split_pos) const noexcept;

    /**
     * @brief fits_in_long_long method that calculates if a number fits in a long long type.
     * @return if number fits in long long type.
     * Uses the length to calculate, since long long max is 9223372036854775807.
     */
    bool fits_in_long_long() const noexcept;

    bool fits_in_long_long_fast() const noexcept;

    /**
     * @brief as_long_long returns a long long representation of this BigNumber object.
     * Throws exeption of type std::out_of_range if the number does not fit.
     * @return this BigNumber as a long long, if possible.
     */
    long long as_long_long() const;

    /**
     * @brief length method to get the length of given number, counting the digits.
     * @return the length of this BigNumber.
     */
    size_t length() const noexcept;

    /**
     * @brief is_odd simple method to se if a BigNumber is odd or not.
     * @return if the number is odd.
     */
    bool is_odd() const noexcept;

    /**
     * @brief is_even just a negative of is_odd.
     * @return if a number is even. Zero is considered even.
     */
    bool is_even() const noexcept;

    /**
     * @brief is_zero methot that tests if a number is zero, faster than Number == BigNumber(0).
     * @return if the object is 0.
     */
    bool is_zero() const noexcept;

    /**
     * @brief is_one method that tests if a number is one, faster than creating an object.
     * @return is this object is 1.
     */
    bool is_one() const noexcept;

    /**
     * @brief is_positive methot that returns if this number is positive or not.
     * @return if this object has positive value or not.
     */
    bool is_positive() const noexcept;

private:
    bool m_positive = true;    // positive by default.
    std::vector<int> m_values; // vector that will hold the digts. vector {1, 2, 3} means number 321.

    /**
     * @brief remove_left_zeroes removes all the zeroes to the left of a number,
     * so 0008 becomes 8 and 000 becomes 0.
     */
    void remove_left_zeroes() noexcept;

    /**
     * @brief do_carry_over utility method for addition, that does the carryover.
     * @param start the point to start the parsing, used to recall the function recursively.
     */
    void do_carry_over(int start = 0) noexcept;

    /**
     * @brief after_operation just a method that updates internal stuff,
     * should be called after most internal value update operations.
     */
    void after_operation() noexcept;

    /**
     * @brief divide method that contains the division logid.
     * @param number to divide by.
     * @return a pair, the first one is the division quocient, the second the division rest.
     * The division rest is ALWAYS positive, since ISO14882:2011 says that the sign of the remainder is implementation-defined.
     * Throws in case of division | module by 0.
     */
    std::pair<BigNumber, BigNumber> divide(const BigNumber &number) const;

    /**
     * @brief BigNumber this is a private constructor just to copy the vector.
     * No checking is done to see if the vector is valid or not.
     * An empty vector will result in the object being init with 0.
     * @param vector the object m_values.
     * @param reversed if true, the vector will be read in the reverse order.
     */
    BigNumber(const std::vector<int> &vector, bool reversed = false) noexcept;
};

#endif // BIGNUMBER_H
