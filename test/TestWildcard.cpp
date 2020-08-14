/**********************************************************************************
 * MIT License
 * 
 * Copyright (c) 2018 Antoine Beauchamp
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************************/

#include "TestWildcard.h"
#include "Wildcard.h"
namespace shellanything { namespace test
{
  std::string WildcardRebuild(const char * pattern, WildcardResult * results, size_t max_results)
  {
    std::string output;
  
    size_t length = strlen(pattern);
    for(size_t i=0; i<length; i++)
    {
      if (IsWildcard(pattern[i]))
      {
        // Find the matching result
        for(size_t j=0; j<max_results; j++)
        {
          const WildcardResult & result = results[j];
          if (result.valid && result.pattern_offset == i)
          {
            output.append(result.value, result.value_length);
            j = max_results+1; // stop searching a matching result
          }
        }
      }
      else
        output.append(1, pattern[i]);
    }

    return output;
  }
  //--------------------------------------------------------------------------------------------------
  void TestWildcard::SetUp()
  {
  }
  //--------------------------------------------------------------------------------------------------
  void TestWildcard::TearDown()
  {
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWildcard, testFindWildcardCharacters)
  {
    // Test nothing is found
    {
      const char * str = "foo";
      size_t elements[50];
      size_t count = FindWildcardCharacters(str, elements, sizeof(elements));
      ASSERT_EQ(0, count);

      // Assert first element is zerorized
      ASSERT_EQ(0, elements[0]);
    }

    // Test sample found
    {
      const char * str = "f?o*o";
      size_t elements[50];
      size_t count = FindWildcardCharacters(str, elements, sizeof(elements));
      ASSERT_EQ(2, count);
      ASSERT_EQ(1, elements[0]);
      ASSERT_EQ(3, elements[1]);
      ASSERT_EQ(0, elements[2]);
    }

    // Test sequence of '*' found
    {
      const char * str = "f****o";
      size_t elements[50];
      size_t count = FindWildcardCharacters(str, elements, sizeof(elements));
      ASSERT_EQ(1, count);
      ASSERT_EQ(1, elements[0]);
      ASSERT_EQ(0, elements[1]);
    }

    // Test 2 sequences of '*' found
    {
      const char * str = "f**?**o";
      size_t elements[50];
      size_t count = FindWildcardCharacters(str, elements, sizeof(elements));
      ASSERT_EQ(3, count);
      ASSERT_EQ(1, elements[0]);
      ASSERT_EQ(3, elements[1]);
      ASSERT_EQ(4, elements[2]);
      ASSERT_EQ(0, elements[3]);
    }

    // Test sequence of '?' found
    {
      const char * str = "f???o";
      size_t elements[50];
      size_t count = FindWildcardCharacters(str, elements, sizeof(elements));
      ASSERT_EQ(3, count);
      ASSERT_EQ(1, elements[0]);
      ASSERT_EQ(2, elements[1]);
      ASSERT_EQ(3, elements[2]);
      ASSERT_EQ(0, elements[3]);
    }

    // Test buffer too small
    {
      const char * str = "f????o";
      size_t elements[2];
      size_t count = FindWildcardCharacters(str, elements, sizeof(elements));
      ASSERT_EQ(2, count);
      ASSERT_EQ(1, elements[0]);
      ASSERT_EQ(2, elements[1]);
    }

    // Test buffer overflow
    {
      const char * str = "f????o";
      size_t elements[40] = {0};
      size_t count = FindWildcardCharacters(str, elements, 2*sizeof(size_t));
      ASSERT_EQ(2, count);
      ASSERT_EQ(1, elements[0]);
      ASSERT_EQ(2, elements[1]);
      ASSERT_EQ(0, elements[2]);
    }

    // Test NULL buffer
    {
      const char * str = "f????o";
      size_t count = FindWildcardCharacters(str, NULL, 45);
      ASSERT_EQ(4, count);
      count = FindWildcardCharacters(str, NULL, 0);
      ASSERT_EQ(4, count);
    }
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWildcard, testValidByDefault)
  {
    //SIMPLE TEST CASES:
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "abcd";
      const char * value = "abcd";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "ab?d";
      const char * value = "abcd";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "?bcd";
      const char * value = "abcd";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "abc?";
      const char * value = "abcd";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "ab*e";
      const char * value = "abcde";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "*cde";
      const char * value = "abcde";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "abc*";
      const char * value = "abcde";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }

    //COMPLEX TEST CASES:
    {
      //all wildcards must be matched
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "abc*f?h*z";
      const char * value = "abcz";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_FALSE( success );
    }
    {
      //wildcard * can be an empty string
      {
        //1
        static const size_t MAX_RESULTS = 50;
        WildcardResult results[MAX_RESULTS];
        const char * wildcard = "abc*d";
        const char * value = "abcd";
        bool success = WildcardSolve(wildcard, value, results, sizeof(results));
        ASSERT_TRUE( success );
        std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
        ASSERT_EQ( rebuild, value );
      }
      {
        //2
        static const size_t MAX_RESULTS = 50;
        WildcardResult results[MAX_RESULTS];
        const char * wildcard = "abcd*";
        const char * value = "abcd";
        bool success = WildcardSolve(wildcard, value, results, sizeof(results));
        ASSERT_TRUE( success );
        std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
        ASSERT_EQ( rebuild, value );
      }
      {
        //3
        static const size_t MAX_RESULTS = 50;
        WildcardResult results[MAX_RESULTS];
        const char * wildcard = "*abcd";
        const char * value = "abcd";
        bool success = WildcardSolve(wildcard, value, results, sizeof(results));
        ASSERT_TRUE( success );
        std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
        ASSERT_EQ( rebuild, value );
      }
      {
        //4
        static const size_t MAX_RESULTS = 50;
        WildcardResult results[MAX_RESULTS];
        const char * wildcard = "abc*?e";
        const char * value = "abcde";
        bool success = WildcardSolve(wildcard, value, results, sizeof(results));
        ASSERT_TRUE( success );
        std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
        ASSERT_EQ( rebuild, value );
      }
    }
    {
      //while checking for * wildcard, the solver must check all possibilities
      //wildcard * must be "defabc" and NOT de
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "abc*fg";
      const char * value = "abcdefabcfg";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
      std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
      ASSERT_EQ( rebuild, value );
    }
    {
      //* followed by ?
      {
        //1
        static const size_t MAX_RESULTS = 50;
        WildcardResult results[MAX_RESULTS];
        const char * wildcard = "abc*??h";
        const char * value = "abcdefgh";
        bool success = WildcardSolve(wildcard, value, results, sizeof(results));
        ASSERT_TRUE( success );
        std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
        ASSERT_EQ( rebuild, value );
      }
      {
        //2
        static const size_t MAX_RESULTS = 50;
        WildcardResult results[MAX_RESULTS];
        const char * wildcard = "abc*??";
        const char * value = "abcdefg";
        bool success = WildcardSolve(wildcard, value, results, sizeof(results));
        ASSERT_TRUE( success );
        std::string rebuild = WildcardRebuild(wildcard, results, sizeof(results));
        ASSERT_EQ( rebuild, value );
      }
    }
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWildcard, testSpecialCases)
  {
    // Test empty string value can only match with '*' pattern
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "*";
      const char * value = "";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
    }
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "******";
      const char * value = "";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
    }

    // Test empty string value can match an empty pattern.
    {
      static const size_t MAX_RESULTS = 50;
      WildcardResult results[MAX_RESULTS];
      const char * wildcard = "";
      const char * value = "";
      bool success = WildcardSolve(wildcard, value, results, sizeof(results));
      ASSERT_TRUE( success );
    }
  }
  //--------------------------------------------------------------------------------------------------

} //namespace test
} //namespace shellanything
