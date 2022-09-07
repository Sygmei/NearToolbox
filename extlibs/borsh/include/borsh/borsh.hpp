#include <cassert>
#include <cmath>
#include <cstring>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <iostream>

#include <borsh/visit_struct.hpp>


namespace BorshCppInternals {
template <typename T>
struct is_string {
    static const bool value = false;
};

template <class T, class Traits, class Alloc>
struct is_string<std::basic_string<T, Traits, Alloc>> {
    static const bool value = true;
};

template <class T>
struct is_container {
    static const bool value = false;
};

template <class T, class Alloc>
struct is_container<std::vector<T, Alloc>> {
    static const bool value = true;
};

template <class T, class Alloc>
struct is_container<std::list<T, Alloc>> {
    static const bool value = true;
};

template <class T, class Alloc>
struct is_container<std::map<T, Alloc>> {
    static const bool value = true;
};

template <class T, class Alloc>
struct is_container<std::unordered_map<T, Alloc>> {
    static const bool value = true;
};

template <class T, class Alloc>
struct is_container<std::set<T, Alloc>> {
    static const bool value = true;
};

template <class T>
struct is_stdarray {
    static const bool value = false;
};

template <class T, std::size_t N>
struct is_stdarray<std::array<T, N>> {
    static const bool value = true;
};

template <class T>
struct is_map {
    static const bool value = false;
};

template <class T, class Alloc>
struct is_map<std::map<T, Alloc>> {
    static const bool value = true;
};

template <class T, class Alloc>
struct is_map<std::unordered_map<T, Alloc>> {
    static const bool value = true;
};

}

class BorshEncoder {
public:
    template <typename T, typename U, typename... Pack, typename=void>
    constexpr BorshEncoder& Encode(const T& value, const U& value2, const Pack&... types)
    {
        Encode(value);
        Encode(value2);
        (Encode(types), ...);
        return *this;
    }

    template <typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr>
    constexpr BorshEncoder& Encode(const T value)
    {
        const size_t typeSize = sizeof(T);
        uint8_t offset = 0;

        for (size_t i = 0; i < typeSize; i++) {
            uint8_t current_byte = (value >> offset) & 0xff;
            push_to_buffer({ current_byte });
            offset += 8;
        }

        return *this;
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
    constexpr BorshEncoder& Encode(const T value)
    {
        assert(!std::isnan(value) || "NaN value found");

        // From https://github.com/naphaso/cbor-cpp/blob/master/src/encoder.cpp
        const void* punny = &value;

        for (size_t i = 0; i < sizeof(T); i++) {
            uint8_t current_byte = *((uint8_t*)punny + i);
            push_to_buffer({ current_byte });
        }

        return *this;
    }

    template <typename T, std::enable_if_t<visit_struct::traits::is_visitable<visit_struct::traits::clean_t<T>>::value>* = nullptr>
    BorshEncoder& Encode(const T& serializable_struct)
    {
        visit_struct::for_each(serializable_struct,
            [this](const char* name, const auto& attribute) {
                std::cout << "Encoding attribute " << name << std::endl;
                Encode(attribute);
            });

        return *this;
    }

    template <typename T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    constexpr BorshEncoder& Encode(const T enum_value)
    {
        push_to_buffer({ static_cast<uint8_t>(enum_value)});

        return *this;
    }

    template <typename T, typename std::enable_if<visit_struct::traits::is_encodable<visit_struct::traits::clean_t<T>>::value>::type* = nullptr>
    BorshEncoder& Encode(const T& encodable_struct)
    {
        auto bytes = visit_struct::traits::encodable<visit_struct::traits::clean_t<T>>::bytes(encodable_struct);
        push_to_buffer(bytes);

        return *this;
    }

    BorshEncoder& Encode(const std::string& str)
    {
        // Write the size of the string as an u32 integer
        Encode(static_cast<uint32_t>(str.size()));
        const std::vector<uint8_t> str_bytes(str.begin(), str.end());
        push_to_buffer(str_bytes);

        return *this;
    }

    BorshEncoder& Encode(const char* str)
    {
        const size_t size = std::strlen(str);
        // Write the size of the string as an u32 integer
        Encode(size);

        const std::vector<uint8_t> str_bytes(str, str + size);
        push_to_buffer(str_bytes);

        return *this;
    }

    template <typename T>
    constexpr BorshEncoder& Encode(const std::initializer_list<T>& initList)
    {
        return Encode(std::pair { initList.begin(), initList.size() });
    }

    template <typename T, size_t ArraySize>
    constexpr BorshEncoder& Encode(const std::array<T, ArraySize>& fixed_size_array)
    {
        for (auto value : fixed_size_array)
        {
            Encode(value);
        }
        return *this;
    }

    template <typename T, typename U>
    constexpr BorshEncoder& Encode(const std::pair<T*, U> cArray)
    {
        return Encode(std::pair { (const T*)cArray.first, cArray.second });
    }

    template <typename T, typename U>
    constexpr BorshEncoder& Encode(const std::pair<const T*, U> cArray)
    {
        auto [array, size] = cArray;

        static_assert(std::is_integral<U>::value || (size > 0), "The size of the c array must be a unsigned integer value");

        if constexpr (std::is_array<T>::value) {
            printf("Es array");
        }

        if constexpr (std::is_integral<T>::value) {
            for (size_t i = 0; i < size; ++i) {
                Encode(*array);
                ++array;
            }
        } else if constexpr (std::is_floating_point<T>::value) {
            for (size_t i = 0; i < size; ++i) {
                Encode(*array);
                ++array;
            }
        } else if constexpr (BorshCppInternals::is_string<T>::value) {
            for (size_t i = 0; i < size; ++i) {
                Encode(*array);
                ++array;
            }
        } else if constexpr (std::is_same<const char*, T>::value) {
            for (size_t i = 0; i < size; ++i) {
                Encode(*array);
                ++array;
            }
        } else {
            assert(false || "The type of the array is not supported");
        }

        return *this;
    }

    template <typename T>
    constexpr BorshEncoder& Encode(const std::vector<T>& vector)
    {
        Encode((uint32_t)vector.size());
        for (const auto& value : vector)
        {
            Encode(value);
        }
        // Encode(std::pair { vector.data(), vector.size() });

        return *this;
    }

    template <typename... Args>
    constexpr BorshEncoder& Encode(const std::variant<Args...>& variant_enum)
    {
        Encode(static_cast<uint8_t>(variant_enum.index()));
        std::visit([this](auto&& value) { Encode(value); }, variant_enum);

        return *this;
    }

    const std::vector<uint8_t>& GetBuffer() const
    {
        return m_buffer;
    }

private:
    std::vector<uint8_t> m_buffer;
    void push_to_buffer(std::vector<uint8_t> data)
    {
        std::cout << "  New segment : ";
        for (auto byte : data)
        {
            std::cout << static_cast<uint32_t>(byte) << ", ";
        }
        std::cout << std::endl;
        m_buffer.insert(m_buffer.end(), data.begin(), data.end());
    }

};

class BorshDecoder {
public:
    template <typename... Types>
    void Decode(const uint8_t* bufferBegin, Types&... retriveValues)
    {
        uint8_t* offset = (uint8_t*)bufferBegin;

        // DecodeInternal(&offset, retriveValues...);
        (DecodeInternal<Types>(&offset, retriveValues), ...);
    }

private:
    template <typename T>
    void DecodeInternal(uint8_t** offset, T& value)
    {
        if constexpr (std::is_integral<T>::value || std::is_floating_point<T>::value) {
            // std::cout << typeid(T).name() << '\n';
            value = *((T*)*(offset));
            (*offset) += sizeof(T);
        } else if constexpr (BorshCppInternals::is_string<T>::value) {
            uint32_t strSize = *((uint32_t*)*(offset));
            (*offset) += 4;

            value = std::string((*offset), ((*offset) + strSize));

            (*offset) += strSize;
        } else {
            assert(false || "The type of the array is not supported");
        }
    }
};
