#include <gtest/gtest.h>
#include <AVL_tree.h>
#include <logger_builder.h>
#include <client_logger_builder.h>
#include <iostream>

logger *create_logger(
        std::vector<std::pair<std::string, logger::severity>> const &output_file_streams_setup,
        bool use_console_stream = true,
        logger::severity console_stream_severity = logger::severity::debug)
{
    std::unique_ptr<logger_builder> builder(new client_logger_builder());

    builder->set_format("[%d %t][%s] %m");

    if (use_console_stream)
    {
        builder->add_console_stream(console_stream_severity);
    }

    for (auto &output_file_stream_setup: output_file_streams_setup)
    {
        builder->add_file_stream(output_file_stream_setup.first, output_file_stream_setup.second);
    }

    logger *built_logger = builder->build();


    return built_logger;
}

bool compare_results(
        std::vector<typename binary_search_tree<int, std::string>::value_type> &expected,
        std::vector<typename binary_search_tree<int, std::string>::value_type> &actual)
{

    if (expected.size() != actual.size())
    {
        return false;
    }

    for (auto&& val : std::views::zip(expected, actual))
    {
        if (std::get<0>(val).first != std::get<1>(val).first)
        {
            return false;
        }

        if (std::get<0>(val).second != std::get<1>(val).second)
        {
            return false;
        }
    }

    return true;
}

template<typename tkey, typename tvalue>
struct test_data
{
    tkey key;
    tvalue value;
    size_t depth, height;

    test_data(size_t dep, tkey k, tvalue v, size_t h) : depth(dep), key(k), value(v), height(h) {}
};

template<typename tkey, typename tvalue>
bool infix_iterator_test(
        AVL_tree<tkey, tvalue> const &tree,
        std::vector<test_data<tkey, tvalue>> &expected_result)
{

    auto end_infix = tree.cend_infix();
    auto it = tree.cbegin_infix();

    auto vec_begin =  expected_result.begin(), vec_end = expected_result.end();

    for (; vec_begin != vec_end; ++vec_begin)
    {
        auto& item = *vec_begin;
        // std::cout << "Comparing values: it.depth()=" << it.depth()
        //   << ", item.depth=" << item.depth
        //   << ", it->first=" << it->first
        //   << ", item.key=" << item.key
        //   << ", it->second=" << it->second
        //   << ", item.value=" << item.value
        //   << ", it.get_height()=" << it.get_height()
        //   << ", item.height=" << item.height << std::endl;
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value || it.get_height() != item.height)
        {
            return false;
        }

        ++it;
    }

    return true;
}

template<
        typename tkey,
        typename tvalue>
bool prefix_iterator_test(
        AVL_tree<tkey, tvalue> const &tree,
        std::vector<test_data<tkey, tvalue>> &expected_result)
{
    auto end_prefix = tree.cend_prefix();
    auto it = tree.cbegin_prefix();

    for (auto const &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value || it.get_height() != item.height)
        {
            return false;
        }

        ++it;
    }

    return true;
}

template<
        typename tkey,
        typename tvalue>
bool postfix_iterator_test(
        AVL_tree<tkey, tvalue> const &tree,
        std::vector<test_data<tkey, tvalue>> &expected_result)
{
    std::string line;
    auto end_postfix = tree.cend_postfix();
    auto it = tree.cbegin_postfix();

    for (auto& item : expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value || it.get_height() != item.height)
        {
            return false;
        }

        ++it;
    }

    return true;
}

TEST(AVLTreePositiveTests, test1)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "AVL_tree_tests_logs.txt",
                logger::severity::trace
            },
        }));
    
    logger->trace("AVLTreePositiveTests.test1 started");
    
    auto avl = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl->emplace(5, "a");
    avl->emplace(2, "b");
    avl->emplace(15, "c");
    avl->emplace(3, "d");
    avl->emplace(14, "e");
    avl->emplace(1, "l");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "l", 1),
            test_data<int, std::string>(1, 2, "b", 2),
            test_data<int, std::string>(2, 3, "d", 1),
            test_data<int, std::string>(0, 5, "a", 3),
            test_data<int, std::string>(2, 14, "e", 1),
            test_data<int, std::string>(1, 15, "c", 2)
        };
    
    EXPECT_TRUE(infix_iterator_test(*avl, expected_result));
    
    logger->trace("AVLTreePositiveTests.test1 finished");
}

TEST(AVLTreePositiveTests, test2)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test2 started");

    auto avl = std::make_unique<AVL_tree<int, int>>(std::less<int>(), nullptr, logger.get());
    
    avl->emplace(1, 5);
    avl->emplace(2, 12);
    avl->emplace(15, 1);
    avl->emplace(3, 67);
    avl->emplace(4, 45);
    
    std::vector<test_data<int, int>> expected_result =
        {
                test_data<int, int>(0, 2, 12, 3),
                test_data<int, int>(1, 1, 5, 1),
                test_data<int, int>(1, 4, 45, 2),
                test_data<int, int>(2, 3, 67, 1),
                test_data<int, int>(2, 15, 1, 1)
        };
    
    EXPECT_TRUE(prefix_iterator_test(*avl, expected_result));
    
    logger->trace("AVLTreePositiveTests.test2 finished");
}

TEST(AVLTreePositiveTests, test3)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test3 started");

    auto avl = std::make_unique<AVL_tree<std::string, int>>(std::less<std::string>(), nullptr, logger.get());
    
    avl->emplace("a", 1);
    avl->emplace("b", 2);
    avl->emplace("c", 15);
    avl->emplace("d", 3);
    avl->emplace("e", 4);
    
    std::vector<test_data<std::string, int>> expected_result =
        {
                test_data<std::string, int>(1, "a", 1, 1),
                test_data<std::string, int>(2, "c", 15, 1),
                test_data<std::string, int>(2, "e", 4, 1),
                test_data<std::string, int>(1, "d", 3, 2),
                test_data<std::string, int>(0, "b", 2, 3)
        };
    
    EXPECT_TRUE(postfix_iterator_test(*avl, expected_result));
    
    logger->trace("AVLTreePositiveTests.test3 finished");
}

TEST(AVLTreePositiveTests, test4)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test4 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());

    avl1->emplace(6, "a");
    avl1->emplace(8, "c");
    avl1->emplace(15, "x");
    avl1->emplace(4, "j");
    avl1->emplace(1, "i");
    avl1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i", 1),
            test_data<int, std::string>(1, 4, "j", 2),
            test_data<int, std::string>(2, 5, "b", 1),
            test_data<int, std::string>(0, 6, "a", 3),
            test_data<int, std::string>(1, 8, "c", 2),
            test_data<int, std::string>(2, 15, "x", 1)
        };
    
    AVL_tree<int, std::string> avl2(std::move(*avl1));
    
    EXPECT_TRUE(infix_iterator_test(avl2, expected_result));
    
    logger->trace("AVLTreePositiveTests.test4 finished");
}

TEST(AVLTreePositiveTests, test5)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test5 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl1->emplace(6, "a");
    avl1->emplace(8, "c");
    avl1->emplace(15, "x");
    avl1->emplace(4, "j");
    avl1->emplace(1, "i");
    avl1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i", 1),
            test_data<int, std::string>(1, 4, "j", 2),
            test_data<int, std::string>(2, 5, "b", 1),
            test_data<int, std::string>(0, 6, "a", 3),
            test_data<int, std::string>(1, 8, "c", 2),
            test_data<int, std::string>(2, 15, "x", 1)
        };
    
    AVL_tree<int, std::string> avl2 = std::move(*avl1);
    
    EXPECT_TRUE(infix_iterator_test(avl2, expected_result));
    
    logger->trace("AVLTreePositiveTests.test5 finished");
}

TEST(AVLTreePositiveTests, test6)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test6 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl1->emplace(6, "a");
    avl1->emplace(8, "c");
    avl1->emplace(15, "x");
    avl1->emplace(4, "j");
    avl1->emplace(1, "i");
    avl1->emplace(5, "b");
    
    avl1->erase(5);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i", 1),
            test_data<int, std::string>(1, 4, "j", 2),
            test_data<int, std::string>(0, 6, "a", 3),
            test_data<int, std::string>(1, 8, "c", 2),
            test_data<int, std::string>(2, 15, "x", 1)
        };
    
    EXPECT_TRUE(infix_iterator_test(*avl1, expected_result));
    
    logger->trace("AVLTreePositiveTests.test6 finished");
}

TEST(AVLTreePositiveTests, test7)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test7 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl1->emplace(6, "a");
    avl1->emplace(8, "c");
    avl1->emplace(15, "x");
    avl1->emplace(4, "j");
    avl1->emplace(3, "i");
    avl1->emplace(2, "l");
    avl1->emplace(5, "b");
    
    avl1->erase(3);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 2, "l", 1),
            test_data<int, std::string>(1, 4, "j", 2),
            test_data<int, std::string>(2, 5, "b", 1),
            test_data<int, std::string>(0, 6, "a", 3),
            test_data<int, std::string>(1, 8, "c", 2),
            test_data<int, std::string>(2, 15, "x", 1)
        };
    
    EXPECT_TRUE(infix_iterator_test(*avl1, expected_result));
    
    logger->trace("AVLTreePositiveTests.test7 finished");
    
}

TEST(AVLTreePositiveTests, test8)
{

    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test8 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl1->emplace(6, "a");
    avl1->emplace(8, "c");
    avl1->emplace(15, "x");
    avl1->emplace(11, "j");
    avl1->emplace(19, "i");
    avl1->emplace(12, "l");
    avl1->emplace(17, "b");
    avl1->emplace(18, "e");
    
    avl1->erase(15);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 6, "a", 1),
            test_data<int, std::string>(1, 8, "c", 2),
            test_data<int, std::string>(0, 11, "j", 4),
            test_data<int, std::string>(2, 12, "l", 2),
            test_data<int, std::string>(3, 17, "b", 1),
            test_data<int, std::string>(1, 18, "e", 3),
            test_data<int, std::string>(2, 19, "i", 1)
        };
    
    EXPECT_TRUE(infix_iterator_test(*avl1, expected_result));
    
    logger->trace("AVLTreePositiveTests.test8 finished");
}

TEST(AVLTreePositiveTests, test9)
{

    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test9 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl1->emplace(6, "a");
    avl1->emplace(8, "c");
    avl1->emplace(15, "x");
    avl1->emplace(11, "j");
    avl1->emplace(19, "i");
    avl1->emplace(12, "l");
    avl1->emplace(17, "b");
    avl1->emplace(18, "e");
    
    avl1->erase(11);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 6, "a", 1),
            test_data<int, std::string>(1, 8, "c", 2),
            test_data<int, std::string>(2, 12, "l", 1),
            test_data<int, std::string>(0, 15, "x", 3),
            test_data<int, std::string>(2, 17, "b", 1),
            test_data<int, std::string>(1, 18, "e", 2),
            test_data<int, std::string>(2, 19, "i", 1)
        };
    
    EXPECT_TRUE(infix_iterator_test(*avl1, expected_result));
    
    logger->trace("AVLTreePositiveTests.test9 finished");
    
}

TEST(AVLTreePositiveTests, test10)
{

    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test10 started");

    auto avl1 = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    avl1->emplace(6, "l");
    avl1->emplace(8, "c");
    avl1->emplace(15, "l");
    avl1->emplace(11, "o");
    avl1->emplace(9, "h");
    avl1->emplace(2, "e");
    avl1->emplace(4, "b");
    avl1->emplace(18, "e");
    
    std::vector<std::string> vector;
    
    vector.push_back(avl1->at(9));
    vector.push_back(avl1->at(2));
    vector.push_back(avl1->at(15));
    vector.push_back(avl1->at(6));
    vector.push_back(avl1->at(11));
    
    std::string actual_result;
    
    for (auto const &item: vector)
    {
        actual_result.append(item + " ");
    }
    
    EXPECT_EQ(actual_result, "h e l l o ");
    
    logger->trace("AVLTreePositiveTests.test10 finished");
    
}

TEST(AVLTreePositiveTests, test11)
{

    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "AVL_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("AVLTreePositiveTests.test11 started");

    auto avl = std::make_unique<AVL_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());

    auto d = avl->begin_prefix();

    avl->emplace(6, "l");
    avl->emplace(8, "c");
    avl->emplace(15, "l");
    avl->emplace(11, "o");
    avl->emplace(9, "h");
    avl->emplace(2, "e");
    avl->emplace(4, "b");
    avl->emplace(18, "e");

    auto b = avl->lower_bound(2);
    auto e = avl->upper_bound(10);
    std::vector<std::pair<const int, std::string>> actual_result(b, e);
    
    std::vector<std::pair<const int, std::string>> expected_result =
        {
            { 2, "e" },
            { 4, "b" },
            { 6, "l" },
            { 8, "c" },
            { 9, "h" }
        };
    
    EXPECT_TRUE(compare_results(expected_result, actual_result));
    
    logger->trace("AVLTreePositiveTests.test11 finished");
}

int main(
    int argc,
    char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}