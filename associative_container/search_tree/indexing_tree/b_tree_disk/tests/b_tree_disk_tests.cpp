#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <fstream>
#include "b_tree_disk.hpp"

void prepare_test_files(const std::string& base_file_path) {

    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());
}



class SerializableInt {
private:
    int value;

public:
    SerializableInt() : value(0) {}
    explicit SerializableInt(int val) : value(val) {}

    void serialize(std::fstream& stream) const {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    static SerializableInt deserialize(std::fstream& stream) {
        SerializableInt result;
        if (stream.read(reinterpret_cast<char*>(&result.value), sizeof(result.value))) {
            return result;
        }
        return SerializableInt();
    }

    size_t serialize_size() const {
        return sizeof(value);
    }

    bool operator<(const SerializableInt& other) const {
        return value < other.value;
    }

    bool operator==(const SerializableInt& other) const {
        return value == other.value;
    }

    int getValue() const {
        return value;
    }
};


class SerializableString {
private:
    std::string value;

public:
    SerializableString() = default;
    explicit SerializableString(std::string val) : value(std::move(val)) {}

    void serialize(std::fstream& stream) const {
        size_t size = value.size();
        stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (size > 0) {
            stream.write(value.c_str(), static_cast<std::streamsize>(size));
        }
    }

    static SerializableString deserialize(std::fstream& stream) {
        SerializableString result;
        size_t size = 0;

        if (!stream.read(reinterpret_cast<char*>(&size), sizeof(size))) {
            return result;
        }

        if (size > 1000000) {
            return result;
        }

        if (size > 0) {
            result.value.resize(size);
            if (!stream.read(&result.value[0], static_cast<std::streamsize>(size))) {
                result.value.clear();
            }
        }

        return result;
    }

    size_t serialize_size() const {
        return sizeof(size_t) + value.size();
    }

    bool operator==(const SerializableString& other) const {
        return value == other.value;
    }

    bool operator==(const std::string& other) const {
        return value == other;
    }

    const std::string& getValue() const {
        return value;
    }
};


struct SerializableCompare {
    bool operator()(const SerializableInt& lhs, const SerializableInt& rhs) const {
        return lhs < rhs;
    }
};

// Базовый тест на вставку и проверку наличия данных
TEST(BTreeDiskTest, BasicTest) {
    std::string base_file_path = "test_btree_basic";
    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());

    try {

        B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 2> tree(base_file_path);


        ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(10), SerializableString("Ten"))));
        ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(5), SerializableString("Five"))));
        ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(15), SerializableString("Fifteen"))));


        auto val10 = tree.at(SerializableInt(10));
        ASSERT_TRUE(val10.has_value());
        EXPECT_EQ(val10.value().getValue(), "Ten");

        auto val5 = tree.at(SerializableInt(5));
        ASSERT_TRUE(val5.has_value());
        EXPECT_EQ(val5.value().getValue(), "Five");

        auto val15 = tree.at(SerializableInt(15));
        ASSERT_TRUE(val15.has_value());
        EXPECT_EQ(val15.value().getValue(), "Fifteen");

        // Удаляем данные
        ASSERT_TRUE(tree.erase(SerializableInt(5)));
        EXPECT_FALSE(tree.at(SerializableInt(5)).has_value());
    }
    catch (const std::exception& e) {
        FAIL() << "Exception thrown: " << e.what();
    }

    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());
}

// Тест сохранения данных и их проверка
TEST(BTreeDiskTest, PersistenceSimpleTest) {
    std::string base_file_path = "test_btree_persist_simple";
    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());

    try {

        {
            B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 2> tree(base_file_path);

            ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(10), SerializableString("Ten"))));
            ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(20), SerializableString("Twenty"))));
        }


        {
            B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 2> tree(base_file_path);

            auto val10 = tree.at(SerializableInt(10));
            ASSERT_TRUE(val10.has_value());
            EXPECT_EQ(val10.value().getValue(), "Ten");

            auto val20 = tree.at(SerializableInt(20));
            ASSERT_TRUE(val20.has_value());
            EXPECT_EQ(val20.value().getValue(), "Twenty");
        }
    }
    catch (const std::exception& e) {
        FAIL() << "Exception thrown: " << e.what();
    }

    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());
}

// Легкий тест для итераторов: получаем begin, end, инкрементим, проверяем равенство
TEST(BTreeDiskTest, SimpleIteratorTest) {
    std::string base_file_path = "test_btree_iter_simple";
    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());

    try {
        B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 2> tree(base_file_path);

        // Вставляем три элемента
        ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(1), SerializableString("One"))));
        ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(2), SerializableString("Two"))));
        ASSERT_TRUE(tree.insert(std::make_pair(SerializableInt(3), SerializableString("Three"))));


        auto begin = tree.begin();


        auto first_value = *begin;
        EXPECT_EQ(first_value.first.getValue(), 1);


        ++begin;
        auto second_value = *begin;
        EXPECT_EQ(second_value.first.getValue(), 2);


        auto end = tree.end();


        EXPECT_NE(begin, end);

        // Инкрементируем ещё раз
        ++begin;
        auto third_value = *begin;
        EXPECT_EQ(third_value.first.getValue(), 3);

        // Инкрементируем последний раз - должны достичь end()
        ++begin;
        EXPECT_EQ(begin, end);
    }
    catch (const std::exception& e) {
        FAIL() << "Exception in simple iterator test: " << e.what();
    }

    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());
}

// Тест итераторов: вставляем 10 элементов, проходим по всем
TEST(BTreeDiskTest, FullIteratorTest) {
    std::string base_file_path = "test_btree_iter_full";
    prepare_test_files(base_file_path);
    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());

    try {
        B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 2> tree(base_file_path);


        for (int i = 1; i <= 10; i++) {
            ASSERT_TRUE(tree.insert(std::make_pair(
                SerializableInt(i),
                SerializableString("Value-" + std::to_string(i))
            ))) << "Failed to insert key " << i;
        }


        EXPECT_NE(tree.begin(), tree.end()) << "Tree should not be empty";


        std::vector<int> collected_keys;
        int count = 0;

        for (auto it = tree.begin(); it != tree.end() && count < 10; ++it, ++count) {
            collected_keys.push_back((*it).first.getValue());
        }


        EXPECT_EQ(collected_keys.size(), 10) << "Should have collected all 10 keys";


        std::vector<int> expected_keys = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        EXPECT_EQ(collected_keys, expected_keys);
    }
    catch (const std::exception& e) {
        FAIL() << "Exception in full iterator test: " << e.what();
    }

    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());
}


// Тест персистентности, вставляем 20 элементов, закрываем, открываем, проверяем данные
// удаляем четные ключи, проверяем, что остались только нечетные
TEST(BTreeDiskTest, PersistenceMediumTest) {
    std::string base_file_path = "test_btree_persist_medium";
    prepare_test_files(base_file_path);
    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());

    try {

        {
            B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 3> tree(base_file_path);


            for (int i = 1; i <= 20; i++) {
                ASSERT_TRUE(tree.insert(std::make_pair(
                    SerializableInt(i),
                    SerializableString("Value-" + std::to_string(i))
                )));
            }
        }


        {
            B_tree_disk<SerializableInt, SerializableString, SerializableCompare, 3> tree(base_file_path);


            for (int i = 1; i <= 20; i++) {
                auto value = tree.at(SerializableInt(i));
                ASSERT_TRUE(value.has_value()) << "Key " << i << " not found after reload";
                EXPECT_EQ(value.value().getValue(), "Value-" + std::to_string(i));
            }


            for (int i = 2; i <= 20; i += 2) {
                ASSERT_TRUE(tree.erase(SerializableInt(i)));
            }


            for (int i = 1; i <= 20; i++) {
                auto value = tree.at(SerializableInt(i));
                if (i % 2 == 0) {
                    EXPECT_FALSE(value.has_value()) << "Even key " << i << " should be deleted";
                } else {
                    EXPECT_TRUE(value.has_value()) << "Odd key " << i << " should exist";
                }
            }
        }
    }
    catch (const std::exception& e) {
        FAIL() << "Exception in persistence medium test: " << e.what();
    }

    std::remove((base_file_path + ".tree").c_str());
    std::remove((base_file_path + ".data").c_str());
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
