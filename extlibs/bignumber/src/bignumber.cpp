#include <limits>
#include <stdexcept>
#include <string>

#include <bignumber/bignumber.hpp>

/**
 * @brief char_to_int function that converts a char to the int it represents.
 * @param c the char to be parsed as int.
 * @return the int contained in c. E.g. '0' returns 0.
 * Throws an exception if the char is not a number.
 */
int char_to_int(char c)
{
    switch (c)
    { // Since values are only 0-9, this is way faster than atoi.
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    default:
        std::string message("[BigNumber] {Digit Parsing} ->  Char at string is not a valid int. Received: ");
        message.push_back(c);
        throw std::invalid_argument(message);
    }
}

BigNumber::BigNumber(std::string string)
{
    if (string.length() > 0)
    {
        if (string.at(0) == '-' || string.at(0) == '+')
        {
            this->m_positive = string.at(0) == '+';
            string = string.substr(1); // remove the signal.
        }
        this->m_values.reserve(string.length());
        for (int i = string.size() - 1; i >= 0; i--)
        { // from end to start, so that string 321 is represented as vector {1, 2, 3}.
            this->m_values.push_back(char_to_int(string[i]));
        }
    }
    else
    {
        this->m_values.push_back(0); // init with 0 by default.
    }
    this->remove_left_zeroes(); // call remove_left_zeroes, and not after_operation since the object has not been completelly initialized.
}

BigNumber::BigNumber()
    : BigNumber("0")
{
} // zero by default.

BigNumber::BigNumber(long long value)
    : BigNumber(std::to_string(value))
{
} // just call the string constructor since its easier to parse.

BigNumber BigNumber::from_binary(std::string binary, bool is_signed) noexcept
{
    BigNumber number(0);
    if (is_signed)
    {
        number.m_positive = binary.at(0) == '0';
        binary = binary.substr(1);
    }

    BigNumber power_two(1);
    for (int i = binary.length() - 1; i >= 0; i--)
    {
        if (binary.at(i) == '1')
        {
            number += power_two;
        }
        power_two *= 2;
    }
    return number;
}

BigNumber BigNumber::random_big_number(int length)
{
    if (length <= 0)
    {
        throw std::invalid_argument("[BigNumber] {Random BigNumber} ->  random_big_number size must be larger or equal to 1.");
    }

    std::stringstream ss;
    std::random_device rand_gen;

    while (ss.tellp() < length)
    { // add random digits until its larger than the desired length.
        ss << rand_gen();
    }

    std::string random_digits = ss.str().substr(0, length); // cut the excess.
    return BigNumber(random_digits);
}

BigNumber BigNumber::random_big_number_in_range(const BigNumber &min_bound, const BigNumber &max_bound)
{
    if (min_bound >= max_bound)
    {
        throw std::invalid_argument("[BigNumber] {Random BigNumber In Range} ->  Lower bound cannot be bigger or equal to higher bound.");
    }
    if (!min_bound.is_positive() || !max_bound.is_positive())
    {
        throw std::invalid_argument("[BigNumber] {Random BigNumber In Range} ->  Only works with positive BigNumbers.");
    }

    BigNumber diff = max_bound - min_bound;

    BigNumber random_diff_range = BigNumber::random_big_number(diff.length() + 1); // + 1 makes sure the generated value is greater then diff.
    BigNumber mod_r = random_diff_range % diff;

    BigNumber random_n = min_bound + mod_r;

    return random_n;
}

BigNumber &BigNumber::operator=(const BigNumber &number) noexcept
{
    this->m_values = number.m_values;
    this->m_positive = number.m_positive;
    return *this;
}

BigNumber BigNumber::operator+() const noexcept
{
    return *this;
}

BigNumber BigNumber::operator-() const noexcept
{
    BigNumber number = *this;
    if (!number.is_zero())
    {
        number.m_positive = !number.is_positive();
    }
    return number;
}

BigNumber BigNumber::operator+(const BigNumber &number) const noexcept
{
    if (this->is_positive() && !number.is_positive())
    {
        return *this - number.absolute_value();
    }
    else if (!this->is_positive() && number.is_positive())
    {
        return -(number - this->absolute_value());
    }

    if (number.is_zero())
    {
        return *this;
    }
    if (this->is_zero())
    {
        return number;
    }
    // At this point, both signs are equal.
    BigNumber result = *this; // not a pointer, since it will hold the result.

    for (size_t i = 0; i < number.length(); i++)
    { // the numbers will be iterated in normal order, units to hundreds.
        int digit = number.m_values[i];
        if (i < result.length())
        {                                // if there is a digit at the same position in the other number.
            result.m_values[i] += digit; // sum to it.
        }
        else
        {
            result.m_values.push_back(digit); // no digit in this position, add to the end of vector.
        }
    }
    result.do_carry_over();
    result.after_operation();
    return result;
}

BigNumber BigNumber::operator-(const BigNumber &number) const noexcept
{
    if (this->is_positive() && !number.is_positive())
    {
        return *this + number.absolute_value();
    }
    else if (!this->is_positive() && number.is_positive())
    {
        return -(number + this->absolute_value());
    }

    if (number.is_zero())
    {
        return *this;
    }
    if (this->is_zero())
    {
        return number;
    }

    // At this point, both signs are equal.
    // To subtract one number from other, it is important to know the larger and the smaller one,
    // since when subtracting 20 from 2 what you do is 20 - 2  and then reverse the signal.
    BigNumber result;         // result is not a pointer, since it will hold the result.
    const BigNumber *smaller; // pointer to the smaller number. Read only.
    if (this->length() >= number.length())
    {
        result = *this;
        smaller = &number;
    }
    else
    {
        result = number;
        smaller = this;
    }

    for (int i = 0; i < smaller->length(); i++)
    { // iterate in normal order, units to hundreds.
        int dif = result.m_values[i] - smaller->m_values[i];
        if (dif < 0)
        { // subtraction cannot be done without borrowing.
            // search for a number to borrow.
            for (int j = i + 1; j < result.length(); j++)
            {
                if (result.m_values[j] == 0)
                { // replace 0's with 9 until finding a non-zero number.
                    result.m_values[j] = 9;
                }
                else
                { // subtract one from it and add 10 to the dif.
                    dif += 10;
                    result.m_values[j]--;
                    break;
                }
            }
        }

        result.m_values[i] = dif;
    }
    result.m_positive = *this >= number; // If this is less than the number being subtracted, result will be negative.
    result.after_operation();
    return result;
}

BigNumber BigNumber::operator*(const BigNumber &number) const noexcept
{
    if (this->is_zero() || number.is_zero())
    {
        return BigNumber(0);
    }
    if (this->is_one())
    {
        return number;
    }
    if (number.is_one())
    {
        return *this;
    }

    if (this->length() < 10 && number.length() < 10)
    { // result can fit in a long long.
        return BigNumber(this->as_long_long() * number.as_long_long());
    }

    int maxlength = std::max(this->length(), number.length());
    int split_point = maxlength / 2; // round down.

    // Apply the Karatsuba algorithm, you should read about it before reading this code.
    std::pair<BigNumber, BigNumber> split_this = this->split_at(split_point);
    std::pair<BigNumber, BigNumber> split_number = number.split_at(split_point);

    BigNumber second_product = split_this.second * split_number.second;
    BigNumber first_product = split_this.first * split_number.first;
    BigNumber sum_product = (split_this.second + split_this.first) * (split_number.second + split_number.first);

    BigNumber first_padded = first_product.times10(split_point * 2);
    BigNumber delta_padded = (sum_product - first_product - second_product).times10(split_point);

    return first_padded + delta_padded + second_product;
}

BigNumber BigNumber::operator/(const BigNumber &number) const
{
    return this->divide(number).first;
}

BigNumber BigNumber::operator%(const BigNumber &number) const
{
    return this->divide(number).second;
}

BigNumber BigNumber::pow(BigNumber number) const
{
    if (this->is_zero() && number.is_zero())
    {
        throw std::invalid_argument("[BigNumber] {Pow} ->  Zero to the power of Zero is undefined.");
    }
    if (!number.is_positive())
    {
        throw std::invalid_argument("[BigNumber] {Pow} ->  Power cannot be negative.");
    }
    if (this->is_zero())
    {
        return BigNumber(0);
    }
    if (number.is_zero())
    {
        return BigNumber(1);
    }
    if (number.is_odd())
    {
        return *this * (*this * *this).pow((number - 1) / 2);
    }
    else
    {
        return (*this * *this).pow(number / 2);
    }
}

BigNumber BigNumber::mod_pow(const BigNumber &power, const BigNumber &mod) const
{
    if (mod.is_zero())
    {
        throw std::invalid_argument("[BigNumber] {Mod Pow} ->  Module by Zero is undefined.");
    }
    if (this->is_zero() && power.is_zero())
    {
        throw std::invalid_argument("[BigNumber] {Mod Pow} ->  Zero to the power of Zero is undefined.");
    }
    if (!power.is_positive())
    {
        throw std::invalid_argument("[BigNumber] {Mod Pow} ->  Power cannot be negative.");
    }
    if (this->is_zero())
    {
        return BigNumber(0);
    }
    BigNumber result(1);
    for (BigNumber i = 0; i < power; i++)
    { // repeat power times.
        result = (*this * result) % mod;
    }
    return result;
}

BigNumber &BigNumber::operator+=(const BigNumber &number) noexcept
{
    *this = *this + number;
    return *this;
}

BigNumber &BigNumber::operator-=(const BigNumber &number) noexcept
{
    *this = *this - number;
    return *this;
}

BigNumber &BigNumber::operator*=(const BigNumber &number) noexcept
{
    *this = *this * number;
    return *this;
}

BigNumber &BigNumber::operator/=(const BigNumber &number)
{
    *this = *this / number;
    return *this;
}

BigNumber &BigNumber::operator%=(const BigNumber &number)
{
    *this = *this % number;
    return *this;
}

BigNumber &BigNumber::operator++() noexcept
{
    *this += 1;
    return *this;
}

BigNumber &BigNumber::operator--() noexcept
{
    *this -= 1;
    return *this;
}

BigNumber BigNumber::operator++(int) noexcept
{
    BigNumber copy = *this;
    *this += 1;
    return copy;
}

BigNumber BigNumber::operator--(int) noexcept
{
    BigNumber copy = *this;
    *this -= 1;
    return copy;
}

bool BigNumber::operator<(const BigNumber &number) const noexcept
{
    if (this->m_positive != number.m_positive)
    { // oposite signs.
        return !this->m_positive;
    }
    if (this->length() != number.length())
    { // not the same length.
        return this->length() < number.length();
    }
    // at this point, both are the same length.
    if (this->m_positive)
    {                                                  // both positives.
        return this->as_string() < number.as_string(); // compare string representation.
    }
    return -(*this) > -number; // both negatives.
}

bool BigNumber::operator>(const BigNumber &number) const noexcept
{
    return *this >= number && !(*this == number);
}

bool BigNumber::operator<=(const BigNumber &number) const noexcept
{
    return *this == number || *this < number;
}

bool BigNumber::operator>=(const BigNumber &number) const noexcept
{
    return !(*this < number);
}

bool BigNumber::operator==(const BigNumber &number) const noexcept
{
    return this->m_positive == number.m_positive && this->m_values == number.m_values;
}

bool BigNumber::operator!=(const BigNumber &number) const noexcept
{
    return !(*this == number);
}

std::string BigNumber::as_string() const noexcept
{
    std::stringstream ss;
    if (!this->is_positive())
    {
        ss << '-';
    }
    for (int i = this->length() - 1; i >= 0; i--)
    { // reverse order, so that vector {1, 2, 3} prints 321 and not 123.
#ifdef BIG_NUMBER_DEBUG
        if (this->m_values[i] < 0 || this->m_values[i] > 9)
        {
            std::cerr << "[BigNumber] {As String} ->  m_values containing invalid value: " << this->m_values[i] << ". Aborting..." << std::endl;
            exit(1);
        }
#endif
        ss << this->m_values[i];
    }
    return ss.str();
}

BigNumber BigNumber::absolute_value() const noexcept
{
    BigNumber temp = *this;
    temp.m_positive = true;
    return temp;
}

BigNumber BigNumber::times10(int times) const noexcept
{
    BigNumber temp = *this;
    for (int i = 0; i < times; i++)
    {
        temp.m_values.insert(temp.m_values.begin(), 0);
    }
    temp.after_operation();
    return temp;
}

BigNumber BigNumber::divide10(int times) const noexcept
{
    if (times >= this->length())
    {
        return BigNumber(0);
    }
    BigNumber divided = *this;
    for (int i = 0; i < times; i++)
    {
        divided.m_values.erase(divided.m_values.begin());
    }
    return divided;
}

std::string BigNumber::as_binary() const noexcept
{
    std::stringstream ss;

    BigNumber copy = this->absolute_value();

    while (copy > 0)
    {
        ss << (copy.is_odd() ? '1' : '0');
        copy /= 2;
    }

    ss << (this->is_positive() ? '0' : '1'); // Big for signed.

    std::string as_string = ss.str();
    std::reverse(as_string.begin(), as_string.end());
    return as_string;
}

std::pair<BigNumber, BigNumber> BigNumber::split_at(long long split_pos) const noexcept
{
    split_pos = std::min(std::max(split_pos, (long long)0), (long long)this->length()); // cap values to [0, lengh]. I miss C++ 17 clamp.

    std::vector<int> first_half;
    first_half.reserve(split_pos);

    std::vector<int> second_half;
    second_half.reserve(this->length() - split_pos);

    for (size_t i = 0; i < this->length(); i++)
    {
        int digit = this->m_values[i];
        if (i > split_pos - 1)
        {
            first_half.push_back(digit);
        }
        else
        {
            second_half.push_back(digit);
        }
    }

    return std::make_pair(BigNumber(first_half), BigNumber(second_half));
}

bool BigNumber::fits_in_long_long() const noexcept
{
    return this->length() <= std::numeric_limits<long long>::max();
}

bool BigNumber::fits_in_long_long_fast() const noexcept
{
    return this->length() < 19;
}

long long BigNumber::as_long_long() const
{
#ifdef BIG_NUMBER_DEBUG
    if (!this->fits_in_long_long())
    {
        std::cerr << "[BigNumber] {As Long Long} Invalid long long:" << *this << std::endl;
        exit(1);
    }
#endif
    return std::stoll(this->as_string());
}

size_t BigNumber::length() const noexcept
{
    return this->m_values.size();
}

bool BigNumber::is_odd() const noexcept
{
    return this->m_values[0] & 1;
}

bool BigNumber::is_even() const noexcept
{
    return !this->is_odd();
}

bool BigNumber::is_zero() const noexcept
{
    return this->length() == 1 && this->m_values[0] == 0;
}

bool BigNumber::is_one() const noexcept
{
    return this->m_positive && this->length() == 1 && this->m_values[0] == 1;
}

bool BigNumber::is_positive() const noexcept
{
    return this->m_positive;
}

// Private

void BigNumber::remove_left_zeroes() noexcept
{
    for (int i = this->length() - 1; i >= 1; i--)
    { // until 1, not 0 so that 0 is represented as {0} and not {}
        if (this->m_values[i] == 0)
        {
            this->m_values.pop_back(); // pops all zeroes to the left of the number.
        }
        else
        {
            break;
        }
    }
}

void BigNumber::do_carry_over(int start) noexcept
{
    for (int i = start; i < this->length(); i++)
    {
        if (this->m_values[i] > 9)
        {
            this->m_values[i] -= 10;
            if (i + 1 < this->length())
            {
                this->m_values[i + 1]++;
            }
            else
            {
                this->m_values.push_back(1);
                return do_carry_over(i);
            }
        }
    }
}

void BigNumber::after_operation() noexcept
{
    this->remove_left_zeroes();
    if (this->is_zero())
    { // prevents -0.
        this->m_positive = true;
    }
#ifdef BIG_NUMBER_DEBUG
    for (int i = 0; i < this->length(); i++)
    {
        if (this->m_values[i] < 0 || this->m_values[i] > 9)
        {
            std::cerr << "[BigNumber] {After Operation} Invalid value in vector: " << this->m_values[i] << std::endl;
            exit(1);
        }
    }
#endif
}

std::pair<BigNumber, BigNumber> BigNumber::divide(const BigNumber &number) const
{
    if (number.is_zero())
    {
        throw std::invalid_argument("[BigNumber] {Divide} ->  Division | Module by 0 is undefined.");
    }
    if (number.is_one())
    {
        return std::make_pair(*this, BigNumber(0));
    }
    if (number == *this)
    {
        return std::make_pair(BigNumber(1), BigNumber(0));
    }
    if (number > *this)
    {
        return std::make_pair(BigNumber(0), *this);
    }
    // At this point, we can assume *this is larger than number.

    if (this->fits_in_long_long_fast() && number.fits_in_long_long_fast())
    { // this makes for huge optization.
        long long ll_this = this->as_long_long();
        long long ll_number = number.as_long_long();
        return std::make_pair(BigNumber(ll_this / ll_number), BigNumber(abs(ll_this % ll_number)));
    }

    BigNumber rest = this->absolute_value(); // this number will be modified.
    const BigNumber absolute_number = number.absolute_value();

    BigNumber quotient;

    // Iterate length_difference times, decreasing.
    int length_difference = rest.length() - absolute_number.length();
    while (length_difference-- >= 0)
    {
        // The number that it will try to subtract will be the absolute_number passed times 10 to the power of the current length_difference.
        // This will make for HUGE permorface, since instead of subtracting 2 from 200 100 times it will subtract 200 from 200 once.
        BigNumber to_subtract = absolute_number.times10(length_difference);
        while (rest >= to_subtract)
        {                                                        // if we can subtract it.
            quotient += BigNumber(1).times10(length_difference); // increase the quotient by the correct number.
            rest -= to_subtract;                                 // subtract the numbers.
        }
    }
    quotient.m_positive = this->m_positive == number.m_positive; // Division signal rule.
    quotient.after_operation();
    rest.after_operation();
    return std::make_pair(quotient, rest);
}

BigNumber::BigNumber(const std::vector<int> &vector, bool reversed) noexcept
{
    if (vector.size() > 0)
    {
#ifdef BIG_NUMBER_DEBUG
        for (int i = 0; i < vector.size(); i++)
        {
            if (vector[i] < 0 || vector[i] > 9)
            {
                std::cerr << "[BigNumber] {Vector Constructor} Invalid value in vector: " << vector[i] << std::endl;
                exit(1);
            }
        }
#endif

        this->m_values = vector;
        if (reversed)
        {
            std::reverse(this->m_values.begin(), this->m_values.end());
        }
    }
    else
    {
        this->m_values.push_back(0);
    }
    this->after_operation();
}