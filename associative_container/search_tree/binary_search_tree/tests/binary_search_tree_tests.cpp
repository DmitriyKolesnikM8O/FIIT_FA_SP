#include <gtest/gtest.h>
#include <binary_search_tree.h>
#include <logger_builder.h>
#include <client_logger_builder.h>
#include <allocator_sorted_list.h>
#include <allocator_boundary_tags.h>
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
    size_t depth;

    test_data(size_t dep, tkey k, tvalue v) : depth(dep), key(k), value(v){}
};

template<typename tkey, typename tvalue>
bool infix_iterator_test(
    binary_search_tree<tkey, tvalue> const &tree,
    std::vector<test_data<tkey, tvalue>> &expected_result)
{
    auto end_infix = tree.cend_infix();
    auto it = tree.cbegin_infix();

    auto vec_begin =  expected_result.begin(), vec_end = expected_result.end();
    
    for (; vec_begin != vec_end; ++vec_begin)
    {
        auto& item = *vec_begin;
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value)
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
    binary_search_tree<tkey, tvalue> const &tree,
    std::vector<test_data<tkey, tvalue>> &expected_result)
{
    auto end_prefix = tree.cend_prefix();
    auto it = tree.cbegin_prefix();
    
    for (auto const &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value)
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
    binary_search_tree<tkey, tvalue> const &tree,
    std::vector<test_data<tkey, tvalue>> &expected_result)
{
    std::string line;
    auto end_postfix = tree.cend_postfix();
    auto it = tree.cbegin_postfix();
    
    for (auto& item : expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value)
        {
            return false;
        }
        
        ++it;
    }
    
    return true;
}

TEST(binarySearchTreePositiveTests, noIteratorTest)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                           {
                                                   {
                                                           "binary_search_tree_tests_logs.txt",
                                                           logger::severity::trace
                                                   }
                                           }));
    logger->trace("binarySearchTreePositiveTests.test1 started");

    // auto al = std::make_unique<allocator_sorted_list>(10000);
    auto al = std::make_unique<allocator_boundary_tags>(10000);

    auto bst = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), al.get(), logger.get());
//    auto bst = new binary_search_tree<int, std::string>(key_comparer(), al.get(), logger.get());

    bst->emplace(5, "a");
    bst->emplace(2, "b");
    bst->emplace(15, "c");
    bst->emplace(3, "d");
    bst->emplace(14, "e");
    bst->emplace(1, "l");

    EXPECT_EQ("b", bst->at(2));
    EXPECT_EQ("e", bst->at(14));
    EXPECT_EQ("l", bst->at(1));

    bst->emplace_or_assign(5, "t");

    EXPECT_EQ("t", bst->at(5));

    ASSERT_THROW(bst->at(144), std::out_of_range);
}

TEST(binarySearchTreePositiveTests, test1)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test1 started");

    auto bst = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst->emplace(5, "a");
    bst->emplace(2, "b");
    bst->emplace(15, "c");
    bst->emplace(3, "d");
    bst->emplace(14, "e");
    bst->emplace(1, "l");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(2, 1, "l"),
                test_data<int, std::string>(1, 2, "b"),
                test_data<int, std::string>(2, 3, "d"),
                test_data<int, std::string>(0, 5, "a"),
                test_data<int, std::string>(2, 14, "e"),
                test_data<int, std::string>(1, 15, "c")
        };
    
    EXPECT_TRUE(infix_iterator_test(*bst, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test1 finished");
}

TEST(binarySearchTreePositiveTests, test2)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "binary_search_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 }
                                                         }));
    logger->trace("binarySearchTreePositiveTests.test2 started");
    
    auto bst = std::make_unique<binary_search_tree<int, int>>(std::less<int>(), pp_allocator<int>(), logger.get());
    
    bst->emplace(1, 5);
    bst->emplace(2, 12);
    bst->emplace(15, 1);
    bst->emplace(3, 67);
    bst->emplace(4, 45);
    
    std::vector<test_data<int, int>> expected_result =
        {
                test_data<int, int>(0, 1, 5),
                test_data<int, int>(1, 2, 12),
                test_data<int, int>(2, 15, 1),
                test_data<int, int>(3, 3, 67),
                test_data<int,int>(4, 4, 45)
        };
    
    EXPECT_TRUE(prefix_iterator_test(*bst, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test2 finished");
}

TEST(binarySearchTreePositiveTests, test3)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test3 started");
    
    auto bst = std::make_unique<binary_search_tree<std::string, int>>(std::less<std::string>(), nullptr, logger.get());
    
    bst->emplace("a", 1);
    bst->emplace("b", 2);
    bst->emplace("c", 15);
    bst->emplace("d", 3);
    bst->emplace("e", 4);
    
    std::vector<test_data<std::string, int>> expected_result =
        {
                test_data<std::string, int>(4, "e", 4),
                test_data<std::string, int>(3, "d", 3),
                test_data<std::string, int>(2, "c", 15),
                test_data<std::string, int>(1, "b", 2),
                test_data<std::string, int>(0, "a", 1)
        };
    
    EXPECT_TRUE(postfix_iterator_test(*bst, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test3 finished");
}

TEST(binarySearchTreePositiveTests, test4)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test4 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst1->emplace(6, "a");
    bst1->emplace(8, "c");
    bst1->emplace(15, "x");
    bst1->emplace(4, "j");
    bst1->emplace(1, "i");
    bst1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(2, 1, "i"),
                test_data<int, std::string>(1, 4, "j"),
                test_data<int, std::string>(2, 5, "b"),
                test_data<int, std::string>(0, 6, "a"),
                test_data<int, std::string>(1, 8, "c"),
                test_data<int, std::string>(2, 15, "x")
        };
    
    binary_search_tree<int, std::string> bst2(std::move(*bst1));
    
    EXPECT_TRUE(infix_iterator_test(bst2, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test4 finished");
}

TEST(binarySearchTreePositiveTests, test5)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test5 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst1->emplace(6, "a");
    bst1->emplace(8, "c");
    bst1->emplace(15, "x");
    bst1->emplace(4, "j");
    bst1->emplace(1, "i");
    bst1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(2, 1, "i"),
                test_data<int, std::string>(1, 4, "j"),
                test_data<int, std::string>(2, 5, "b"),
                test_data<int, std::string>(0, 6, "a"),
                test_data<int, std::string>(1, 8, "c"),
                test_data<int, std::string>(2, 15, "x"),
        };
    
    binary_search_tree<int, std::string> bst2 = std::move(*bst1);
    EXPECT_TRUE(infix_iterator_test(bst2, expected_result));
    
    expected_result.clear();
    EXPECT_TRUE(infix_iterator_test(*bst1, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test5 finished");
}

TEST(binarySearchTreePositiveTests, test6)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test6 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst1->emplace(6, "a");
    bst1->emplace(8, "c");
    bst1->emplace(15, "x");
    bst1->emplace(4, "j");
    bst1->emplace(1, "i");
    bst1->emplace(5, "b");
    
    bst1->erase(5);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(2, 1, "i"),
                test_data<int, std::string>(1, 4, "j"),
                test_data<int, std::string>(0, 6, "a"),
                test_data<int, std::string>(1, 8, "c"),
                test_data<int, std::string>(2, 15, "x")
        };
    
    EXPECT_TRUE(infix_iterator_test(*bst1, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test6 finished");
}

TEST(binarySearchTreePositiveTests, test7)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test7 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst1->emplace(6, "a");
    bst1->emplace(8, "c");
    bst1->emplace(15, "x");
    bst1->emplace(4, "j");
    bst1->emplace(3, "i");
    bst1->emplace(2, "l");
    bst1->emplace(5, "b");
    
    bst1->erase(3);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(2, 2, "l"),
                test_data<int, std::string>(1, 4, "j"),
                test_data<int, std::string>(2, 5, "b"),
                test_data<int, std::string>(0, 6, "a"),
                test_data<int, std::string>(1, 8, "c"),
                test_data<int, std::string>(2, 15, "x")
        };
    
    EXPECT_TRUE(infix_iterator_test(*bst1, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test7 finished");
}

TEST(binarySearchTreePositiveTests, test8)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test8 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());

    bst1->emplace(6, "a");
    bst1->emplace(8, "c");
    bst1->emplace(15, "x");
    bst1->emplace(11, "j");
    bst1->emplace(19, "i");
    bst1->emplace(12, "l");
    bst1->emplace(17, "b");
    bst1->emplace(18, "e");
    
    bst1->erase(15);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(0, 6, "a"),
                test_data<int, std::string>(1, 8, "c"),
                test_data<int, std::string>(3, 11, "j"),
                test_data<int, std::string>(2, 12, "l"),
                test_data<int, std::string>(4, 17, "b"),
                test_data<int, std::string>(5, 18, "e"),
                test_data<int, std::string>(3, 19, "i")
        };
    
    EXPECT_TRUE(infix_iterator_test(*bst1, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test8 finished");
}

TEST(binarySearchTreePositiveTests, test9)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test9 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst1->emplace(6, "a");
    bst1->emplace(8, "c");
    bst1->emplace(15, "x");
    bst1->emplace(11, "j");
    bst1->emplace(19, "i");
    bst1->emplace(12, "l");
    bst1->emplace(17, "b");
    bst1->emplace(18, "e");
    
    bst1->erase(6);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(0, 8, "c"),
                test_data<int, std::string>(2, 11, "j"),
                test_data<int, std::string>(3, 12, "l"),
                test_data<int, std::string>(1, 15, "x"),
                test_data<int, std::string>(3, 17, "b"),
                test_data<int, std::string>(4, 18, "e"),
                test_data<int, std::string>(2, 19, "i")
        };
    
    EXPECT_TRUE(infix_iterator_test(*bst1, expected_result));
    
    logger->trace("binarySearchTreePositiveTests.test9 finished");
}

TEST(binarySearchTreePositiveTests, test10)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "binary_search_tree_tests_logs.txt",
                logger::severity::trace
            }
        }));
    logger->trace("binarySearchTreePositiveTests.test10 started");
    
    auto bst1 = std::make_unique<binary_search_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    bst1->emplace(6, "l");
    bst1->emplace(8, "c");
    bst1->emplace(15, "l");
    bst1->emplace(11, "o");
    bst1->emplace(9, "h");
    bst1->emplace(2, "e");
    bst1->emplace(4, "b");
    bst1->emplace(18, "e");
    
    std::vector<std::string> vector;
    
    vector.push_back(bst1->at(9));
    vector.push_back(bst1->at(2));
    vector.push_back(bst1->at(15));
    vector.push_back(bst1->at(6));
    vector.push_back(bst1->at(11));
    
    std::string actual_result;
    
    for (auto const &item: vector)
    {
        actual_result.append(item + " ");
    }
    
    EXPECT_EQ(actual_result, "h e l l o ");
    
    logger->trace("binarySearchTreePositiveTests.test10 finished");
}


int main(
    int argc,
    char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}