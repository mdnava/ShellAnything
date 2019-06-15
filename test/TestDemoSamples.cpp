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

#include "TestDemoSamples.h"
#include "PropertyManager.h"
#include "shellanything/ActionPrompt.h"

namespace shellanything { namespace test
{
  //--------------------------------------------------------------------------------------------------
  void TestDemoSamples::SetUp()
  {
    PropertyManager & pmgr = PropertyManager::getInstance();
    pmgr.clear();
  }
  //--------------------------------------------------------------------------------------------------
  void TestDemoSamples::TearDown()
  {
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestDemoSamples, DISABLED_demoActionPromptQuestion)
  {
    ActionPrompt prompt;
    prompt.setName("myproperty");
    prompt.setTitle("What is your name?");
    prompt.setDefault("Isabelle Ringing");

    Context c;

    bool result = prompt.execute(c);
    
    ASSERT_TRUE(result);
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestDemoSamples, DISABLED_demoActionPromptYesNo)
  {

    ActionPrompt prompt;
    prompt.setType("yesno");
    prompt.setValueYes("yes");
    prompt.setValueNo("no");
    prompt.setName("myproperty");
    prompt.setTitle("Are you afraid of the dark?");
    
    Context c;

    bool result = prompt.execute(c);
    
    ASSERT_TRUE(result);

    //assert user have answered "yes"
    PropertyManager & pmgr = PropertyManager::getInstance();
    std::string myproperty = pmgr.getProperty("myproperty");
    ASSERT_TRUE(myproperty == prompt.getValueYes() || myproperty == prompt.getValueNo());
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestDemoSamples, DISABLED_demoActionPromptOk)
  {
 
    ActionPrompt prompt;
    prompt.setType("ok");
    prompt.setName("myproperty");
    prompt.setTitle("Wait for the system to load and press OK button.");
   
    Context c;
 
    bool result = prompt.execute(c);
   
    ASSERT_TRUE(result);
    ASSERT_TRUE(prompt.isOkQuestion());
 
    //assert property is unchanged
    PropertyManager & pmgr = PropertyManager::getInstance();
    pmgr.setProperty("myproperty", "test");
    std::string myproperty = pmgr.getProperty("myproperty");
    ASSERT_EQ( std::string("test"), myproperty );
  }
  //--------------------------------------------------------------------------------------------------

} //namespace test
} //namespace shellanything
