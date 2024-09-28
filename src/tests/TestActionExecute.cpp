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

#include "TestActionExecute.h"
#include "SelectionContext.h"
#include "ActionExecute.h"
#include "PropertyManager.h"
#include "ConfigManager.h"
#include "ActionManager.h"
#include "Workspace.h"
#include "QuickLoader.h"
#include "ArgumentsHandler.h"
#include "SaUtils.h"

#include "rapidassist/testing.h"
#include "rapidassist/filesystem_utf8.h"
#include "rapidassist/user.h"
#include "rapidassist/timing.h"
#include "rapidassist/environment.h"
#include "rapidassist/process.h"
#include "rapidassist/cli.h"
#include "rapidassist/random.h"
#include "rapidassist/errors.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>
#undef GetEnvironmentVariable
#undef DeleteFile
#undef CreateDirectory
#undef CopyFile
#undef CreateFile
#undef GetCurrentDirectory


namespace shellanything
{
  namespace test
  {
    ra::strings::StringVector RunProcesAndCaptureOutput(const std::string& base_command)
    {
      static const ra::strings::StringVector EMPTY_LIST;

      std::string random_value = ra::strings::ToString(ra::random::GetRandomInt(0, 32767) + 10000);
      std::string temp_file = ra::filesystem::GetTemporaryDirectory() + "\\" + __FUNCTION__ + "." + random_value + ".tmp";
      ra::strings::Replace(temp_file, "::", ".");

      // Execute
      std::string full_command = base_command + ">" + temp_file;
      int exit_code = system(full_command.c_str());

      if (exit_code != 0)
        return EMPTY_LIST; // error

      ra::strings::StringVector output;
      bool success_read = ra::filesystem::ReadTextFile(temp_file, output);
      if (!success_read)
        return EMPTY_LIST;

      return output;
    }

    size_t FindValue(const ra::strings::StringVector& list_values, const std::string& query_value)
    {
      for (size_t i = 0; i < list_values.size(); i++)
      {
        const std::string& list_value = list_values[i];
        if (query_value == list_value)
          return i;
      }
      return std::string::npos;
    }

    ra::strings::StringVector FindNewValues(const ra::strings::StringVector& baseline_values, const ra::strings::StringVector& current_values)
    {
      ra::strings::StringVector new_values;

      for (size_t i = 0; i < current_values.size(); i++)
      {
        const std::string& current_value = current_values[i];
        size_t find_pos = FindValue(baseline_values, current_value);
        if (find_pos == std::string::npos)
        {
          // This is a new value
          new_values.push_back(current_value);
        }
      }

      return new_values;
    }

    void KillCalculatorProcess()
    {
      printf("Killing all calculator processes...\n");

      system("cmd.exe /c taskkill /IM calc.exe >NUL 2>NUL");

      // On Windows 10, calc.exe launches Calculator which is an application in the Microsoft App Store.
      //The executable path is something similar to C:\Program Files\WindowsApps\Microsoft.WindowsCalculator_10.2103.8.0_x64__8wbfmf6g6wwcr\Calculator.exe
      system("cmd.exe /c WMIC PROCESS WHERE \"ExecutablePath like '%%Microsoft.WindowsCalculator%%Calculator.exe'\"    DELETE >NUL 2>NUL");
      system("cmd.exe /c WMIC PROCESS WHERE \"ExecutablePath like '%%Microsoft.WindowsCalculator%%CalculatorApp.exe'\" DELETE >NUL 2>NUL");

      ra::timing::Millisleep(1000);

      printf("killed.\n");
    }

    bool StartCalculatorProcess(ra::process::processid_t & pId)
    {
      // https://stackoverflow.com/questions/63990787/the-process-id-returned-by-the-createprocess-function-is-different-from-the-task
      // calc.exe is a stub/proxy which will launch the actual CalculatorApp.exe.
      // For my system, thats is "C:\Program Files\WindowsApps\Microsoft.WindowsCalculator_11.2405.2.0_x64__8wekyb3d8bbwe\CalculatorApp.exe"
      // However, a normal user cannot launch this process:
      //   Windows cannot access the specified device, path, or file.You may not have the appropriate permissions to access the item.
      // So we hack our way and try our best to detect it...
      //


      /*
      PropertyManager& pmgr = PropertyManager::GetInstance();
      printf("Starting calc.exe...\n");

      SelectionContext c;
      {
        StringList elements;
        elements.push_back("C:\\Windows\\System32\\cmd.exe");
        c.SetElements(elements);
      }

      ActionExecute ae;
      ae.SetPath("C:\\Windows\\System32\\calc.exe");
      ae.SetPid("tmp.pid");

      bool executed = ae.Execute(c);
      if (!executed)
      {
        printf("error: not started.\n");
        return false;
      }

      ra::timing::Millisleep(2000);

      bool parsed = ra::strings::Parse(pmgr.GetProperty("tmp.pid"), pId);
      if (!parsed)
      {
        printf("error: unknown pid.\n");
        return false;
      }

      printf("started.\n");
      return true;
      */

      printf("Starting calc.exe...\n");

      static const std::string base_command = "powershell -ExecutionPolicy bypass -Command \"Get-Process | Where{ $_.ProcessName -eq 'CalculatorApp' } | Select -ExpandProperty 'Id'\"";
      ra::strings::StringVector baseline_processes = RunProcesAndCaptureOutput(base_command);

      system("start \"\" calc.exe >NUL 2>NUL");

      double start = ra::timing::GetMillisecondsTimer();
      double elapsed_ms = ra::timing::GetMillisecondsTimer() - start;
      while (pId == 0 && elapsed_ms < 2500)
      {
        ra::timing::Millisleep(250);

        // Search for a new process
        ra::strings::StringVector new_processes = RunProcesAndCaptureOutput(base_command);

        // Check for new values
        ra::strings::StringVector new_values = FindNewValues(baseline_processes, new_processes);
        if (new_values.size() >= 1)
        {
          // This is the one
          std::string pid_str = new_values[0];

          bool parsed = ra::strings::Parse(pid_str, pId);
          if (parsed)
          {
            printf("started.\n");
            return true;
          }

          // We failed parsing, ignore this value for the next pass.
          baseline_processes.push_back(pid_str);
        }

        // refresh timers
        elapsed_ms = ra::timing::GetMillisecondsTimer() - start;
      }

      printf("error: not running.\n");
      return false;
    }

    void KillShellAnythingArgumentsDebuggerProcess()
    {
      printf("Killing all arguments.debugger.window.exe processes...\n");

      if (ra::environment::IsConfigurationDebug())
        system("cmd.exe /c taskkill /IM arguments.debugger.window-d.exe >NUL 2>NUL");
      else
        system("cmd.exe /c taskkill /IM arguments.debugger.window.exe   >NUL 2>NUL");

      ra::timing::Millisleep(1000);

      printf("killed.\n");
    }

    bool StartShellAnythingArgumentsDebuggerProcess(ra::process::processid_t& pId)
    {
      printf("Starting arguments.debugger.window.exe...\n");

      // Build path of arguments.debugger.window.exe
      std::string current_process_dir = ra::process::GetCurrentProcessDir();
      std::string process_path;
      if (ra::environment::IsConfigurationDebug())
        process_path = current_process_dir + "\\arguments.debugger.window-d.exe";
      else
        process_path = current_process_dir + "\\arguments.debugger.window.exe";

      // Assert that file exists
      if (!ra::filesystem::FileExists(process_path.c_str()))
      {
        printf("Start process failed. File not found: '%s'.\n", process_path.c_str());
        return false;
      }

      // Start the actual process
      ra::process::processid_t tmp_pid = ra::process::StartProcess(process_path, current_process_dir.c_str());

      // Asser created properly
      if (tmp_pid == 0)
      {
        ra::errors::errorcode_t last_error = ra::errors::GetLastErrorCode();
        std::string error_desc = ra::errors::GetErrorCodeDescription(last_error);

        printf("Start process failed. Error code %s, %s.\n", ToHexString(last_error).c_str(), error_desc.c_str());
        return false;
      }

      ra::timing::Millisleep(2000);

      printf("started.\n");
      pId = tmp_pid;
      return true;
    }

    void KillMsPaintProcess()
    {
      printf("Killing all mspaint.exe processes...\n");

      system("cmd.exe /c taskkill /IM mspaint.exe >NUL 2>NUL");
      ra::timing::Millisleep(1000);

      printf("killed.\n");
    }

    bool StartMsPaintProcess(ra::process::processid_t& pId)
    {
      printf("Starting mspaint.exe...\n");

      // Build path of arguments.debugger.window.exe
      std::string current_process_dir = ra::process::GetCurrentProcessDir();
      std::string process_path = "C:\\Windows\\System32\\mspaint.exe";

      // Assert that file exists
      if (!ra::filesystem::FileExists(process_path.c_str()))
      {
        printf("Start process failed. File not found: '%s'.\n", process_path.c_str());
        return false;
      }

      // Start the actual process
      ra::process::processid_t tmp_pid = ra::process::StartProcess(process_path, current_process_dir.c_str());

      // Asser created properly
      if (tmp_pid == 0)
      {
        ra::errors::errorcode_t last_error = ra::errors::GetLastErrorCode();
        std::string error_desc = ra::errors::GetErrorCodeDescription(last_error);

        printf("Start process failed. Error code %s, %s.\n", ToHexString(last_error).c_str(), error_desc.c_str());
        return false;
      }

      ra::timing::Millisleep(2000);

      printf("started.\n");
      pId = tmp_pid;
      return true;
    }

    namespace FindProcessWindows
    {
      static const uint32_t ERROR_PROCESS_NOT_FOUND = (uint32_t)-1;

      struct DATA
      {
        uint32_t pId;
        bool found; // true when the given process id was found
        bool error;
        std::vector<HWND> windows;
      };

      BOOL CALLBACK enum_windows_callback(HWND hWnd, LPARAM lParam)
      {
        DATA& data = *(DATA*)lParam;

        //// Debugging
        //int length = GetWindowTextLength(handle);
        //char* buffer = new char[length + 1];
        //GetWindowTextA(handle, buffer, length + 1);
        //std::string windowTitle(buffer);
        //delete[] buffer;
        //printf("%s\n", windowTitle.c_str());

        // Get the process of the current window
        DWORD owner_process_id = 0;
        DWORD threadId = GetWindowThreadProcessId(hWnd, &owner_process_id);
        if (threadId == 0 || owner_process_id == 0)
        {
          data.error = true;
          return FALSE; // stop enumeration
        }

        // Is that our queried process
        if (data.pId == owner_process_id)
        {
          data.found = true;
          data.windows.push_back(hWnd);
        }
        return TRUE; // continue enumeration
      }

      uint32_t GetProcessWindowsCount(uint32_t pId)
      {
        DATA data;
        data.pId = pId;
        data.found = false;
        data.error = false;
        data.windows.clear();

        EnumWindows(enum_windows_callback, (LPARAM)&data);

        if (data.error)
          return ERROR_PROCESS_NOT_FOUND;
        if (!data.found)
          return 0; // process not found while enumerating windows
        uint32_t count = (uint32_t)data.windows.size();
        return count;
      }
    }

    //--------------------------------------------------------------------------------------------------
    void TestActionExecute::SetUp()
    {
    }
    //--------------------------------------------------------------------------------------------------
    void TestActionExecute::TearDown()
    {
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testBasic)
    {
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows\\System32\\calc.exe");
      c.SetElements(elements);

      c.RegisterProperties();

      //Execute the action
      ActionExecute ae;
      ae.SetPath("C:\\Windows\\System32\\mspaint.exe");
      ae.SetBaseDir("");
      ae.SetArguments("");

      bool executed = ae.Execute(c);
      ASSERT_TRUE(executed);

      //Cleanup
      ra::timing::Millisleep(500);
      KillMsPaintProcess();
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testBaseDir)
    {
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows\\System32\\calc.exe");
      c.SetElements(elements);

      c.RegisterProperties();

      std::string home_dir = ra::user::GetHomeDirectory();

      //Execute the action
      ActionExecute ae;
      ae.SetPath("mspaint.exe");
      ae.SetBaseDir("C:\\Windows\\System32");
      ae.SetArguments("");

      bool executed = ae.Execute(c);
      ASSERT_TRUE(executed);

      //Cleanup
      ra::timing::Millisleep(500);
      KillCalculatorProcess();
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testArguments)
    {
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows\\System32\\calc.exe");
      c.SetElements(elements);

      c.RegisterProperties();

      std::string home_dir = ra::user::GetHomeDirectory();
      std::string temp_dir = ra::filesystem::GetTemporaryDirectory();
      std::string destination_path = temp_dir + "\\my_calc.exe";
      std::string arguments = "/c copy C:\\Windows\\System32\\calc.exe " + destination_path + ">NUL 2>NUL";

      //Execute the action
      ActionExecute ae;
      ae.SetPath("cmd.exe");
      ae.SetBaseDir(temp_dir);
      ae.SetArguments(arguments);

      bool executed = ae.Execute(c);
      ASSERT_TRUE(executed);

      //Wait for the copy to complete, with a timeout
      static const double timeout_time = 50; //seconds
      bool file_copied = false;
      double timer_start = ra::timing::GetMillisecondsTimer();
      double time_elapsed = ra::timing::GetMillisecondsTimer() - timer_start;
      while (!file_copied && time_elapsed <= timeout_time)
      {
        file_copied = ra::filesystem::FileExists(destination_path.c_str());
        ra::timing::Millisleep(500); //allow process to complete
        time_elapsed = ra::timing::GetMillisecondsTimer() - timer_start; //evaluate elapsed time again
      }

      //Validate arguments
      ASSERT_TRUE(file_copied);

      //Cleanup
      ra::filesystem::DeleteFileUtf8(destination_path.c_str());
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testVerb)
    {
      //--------------------------------------------------------------------
      //Note:
      //  The cmd.exe cannot be used for testing the verb and the basedir functionalities at the same time.
      //  Whenever cmd.exe detects it's running elevated, it ignores its launch parameters and always starts in %SystemRoot%\System32. You cannot override this behavior.
      //  For this reason, a command such as `net session >nul 2>&1` cannot be used to detect if the process is elevated or not.
      //
      //  See the following for details:
      //  https://stackoverflow.com/questions/38033790/shellexecute-a-cmd-prompt-with-a-specific-working-directory
      //--------------------------------------------------------------------


      //Skip this test if run on AppVeyor as it requires administrative (elevated) privileges.
      if (ra::testing::IsAppVeyor() ||
          ra::testing::IsJenkins() ||
          ra::testing::IsTravis() ||
          ra::testing::IsGitHubActions())
      {
        printf("Skipping tests as it requires administrative (elevated) privileges.\n");
        return;
      }

      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows\\System32\\calc.exe");
      c.SetElements(elements);

      c.RegisterProperties();

      std::string self_path = ra::process::GetCurrentProcessPath();
      std::string temp_dir = ra::filesystem::GetTemporaryDirectory();
      std::string output_filename = "sa.tests.ProcessSettings.txt";
      std::string output_file_path = temp_dir + "\\" + output_filename;
      std::string arguments = "--PrintProcessSettings --foobar";

      //Cleanup
      ra::filesystem::DeleteFileUtf8(output_file_path.c_str());

      //Execute the action
      ActionExecute ae;
      ae.SetPath(self_path);
      ae.SetBaseDir(temp_dir);
      ae.SetArguments(arguments);
      ae.SetVerb("runas");

      bool executed = ae.Execute(c);
      ASSERT_TRUE(executed);

      //Wait for the operation to complete, with a timeout
      static const double timeout_time = 50; //seconds
      bool result_file_found = false;
      double timer_start = ra::timing::GetMillisecondsTimer();
      double time_elapsed = ra::timing::GetMillisecondsTimer() - timer_start;
      while (!result_file_found && time_elapsed <= timeout_time)
      {
        result_file_found = ra::filesystem::FileExists(output_file_path.c_str());
        ra::timing::Millisleep(500); //allow process to complete
        time_elapsed = ra::timing::GetMillisecondsTimer() - timer_start; //evaluate elapsed time again
      }

      //Validate arguments
      ASSERT_TRUE(result_file_found);

      //Read the result file, search for the elevated process signature.
      std::string result;
      ASSERT_TRUE(ra::filesystem::ReadTextFile(output_file_path, result));
      static const std::string PROCESS_ELEVATED_SIGNATURE = "process.elevated=1";
      ASSERT_NE(result.find(PROCESS_ELEVATED_SIGNATURE), std::string::npos);

      //Cleanup
      ra::filesystem::DeleteFileUtf8(output_file_path.c_str());
    }
    //--------------------------------------------------------------------------------------------------
    int PrintCountdown(int argc, char** argv)
    {
      //MessageBox(NULL, "ATTACH NOW!", "ATTACH NOW!", MB_OK);

      const char* argument_name;

      argument_name = "seconds";
      std::string seconds_str;
      bool has_icon_path = ra::cli::ParseArgument(argument_name, seconds_str, argc, argv);
      if (!has_icon_path)
      {
        printf("Missing argument: '%s'\n", argument_name);
        return 10;
      }

      int seconds = 0;
      bool parsed = ra::strings::Parse(seconds_str, seconds);
      if (!parsed)
      {
        std::cout << "Failed parsing value '" << seconds_str << "'.\n";
        return 1;
      }

      // Start countdown
      std::cout << "Countdown from " << seconds << " to 0.\n";
      for (int i = seconds; i >= 0; i--)
      {
        std::cout << i << "\n";
        Sleep(1000);
      }

      return 0;
    }
    COMMAND_LINE_ENTRY_POINT* PrintCountdownEntryPoint = shellanything::RegisterCommandLineEntryPoint("PrintCountdown", PrintCountdown);
    TEST_F(TestActionExecute, testConsole)
    {
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows\\System32\\calc.exe");
      c.SetElements(elements);

      c.RegisterProperties();

      std::string self_path = ra::process::GetCurrentProcessPath();
      std::string temp_dir = ra::filesystem::GetTemporaryDirectory();
      std::string arguments = "--PrintCountdown --seconds=5";

      // launch the process using all different methods
      for (int test_id = 0; test_id < 3; test_id++)
      {
        //Cleanup
        pmgr.ClearProperty("process.id");

        //Execute the action
        ActionExecute ae;
        ae.SetPath(self_path);
        ae.SetBaseDir(temp_dir);
        ae.SetArguments(arguments);
        ae.SetPid("process.id");

        // launch the process using all different methods
        switch (test_id)
        {
          case 0:
            ae.SetConsole("");
            break;
          case 1:
            ae.SetConsole("false");
            break;
          case 2:
            ae.SetConsole("false");
            ae.SetVerb("open");
            break;
        }

        bool executed = ae.Execute(c);
        ASSERT_TRUE(executed);

        //Wait for the operation to start
        ra::timing::Millisleep(1100);

        // ASSERT the property for the process id is created
        ASSERT_TRUE(pmgr.HasProperty("process.id"));
        std::string pId_str = pmgr.GetProperty("process.id");

        // ASSERT convert pId from string to uint32_t
        uint32_t pId = 0;
        bool parsed = ra::strings::Parse(pId_str, pId);
        ASSERT_TRUE(parsed);

        // ASSERT how many window associated to the new process
        uint32_t count = FindProcessWindows::GetProcessWindowsCount(pId);
        if (count == (uint32_t)-1)
        switch (test_id)
        {
        case 0:
          ASSERT_GE(count, (uint32_t)1);
          break;
        case 1:
          ASSERT_EQ(count, (uint32_t)0);
          break;
        case 2:
          ASSERT_EQ(count, (uint32_t)0);
          break;
        }

        //Cleanup
        ra::process::Kill(pId);
        pmgr.ClearProperty("process.id");
      }
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testProcessId)
    {
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows\\System32\\calc.exe");
      c.SetElements(elements);

      c.RegisterProperties();

      std::string self_path = ra::process::GetCurrentProcessPath();
      std::string temp_dir = ra::filesystem::GetTemporaryDirectory();
      std::string output_filename = "sa.tests.ProcessSettings.txt";
      std::string output_file_path = temp_dir + "\\" + output_filename;
      std::string arguments = "--PrintProcessSettings --foobar";

      //Cleanup
      ra::filesystem::DeleteFileUtf8(output_file_path.c_str());
      pmgr.ClearProperty("my.process.id");

      //Execute the action
      ActionExecute ae;
      ae.SetPath(self_path);
      ae.SetBaseDir(temp_dir);
      ae.SetArguments(arguments);
      ae.SetPid("my.process.id");

      bool executed = ae.Execute(c);
      ASSERT_TRUE(executed);

      //Wait for the operation to complete, with a timeout
      static const double timeout_time = 50; //seconds
      bool result_file_found = false;
      double timer_start = ra::timing::GetMillisecondsTimer();
      double time_elapsed = ra::timing::GetMillisecondsTimer() - timer_start;
      while (!result_file_found && time_elapsed <= timeout_time)
      {
        result_file_found = ra::filesystem::FileExists(output_file_path.c_str());
        ra::timing::Millisleep(500); //allow process to complete
        time_elapsed = ra::timing::GetMillisecondsTimer() - timer_start; //evaluate elapsed time again
      }

      //Validate arguments
      ASSERT_TRUE(result_file_found);

      // ASSERT the property for the process id is created
      ASSERT_TRUE(pmgr.HasProperty("my.process.id"));
      std::string pId = pmgr.GetProperty("my.process.id");

      //Read the result file, search the process id.
      std::string result;
      ASSERT_TRUE(ra::filesystem::ReadTextFile(output_file_path, result));
      const std::string PROCESS_ID_SIGNATURE = std::string("process.pid=") + pId;
      ASSERT_NE(result.find(PROCESS_ID_SIGNATURE), std::string::npos);

      //Cleanup
      ra::filesystem::DeleteFileUtf8(output_file_path.c_str());
      pmgr.ClearProperty("my.process.id");
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, DISABLED_demoAdminCommandPromptHere)
    {
      //----------------------------------
      //Note:
      //  This is not actually a test but a demontration 
      //  of a "Open Adminitrator command prompt here" command.
      //----------------------------------


      //Skip this test if run on AppVeyor as it requires administrative (elevated) privileges.
      if (ra::testing::IsAppVeyor() ||
          ra::testing::IsJenkins() ||
          ra::testing::IsTravis())
      {
        printf("Skipping tests as it requires administrative (elevated) privileges.\n");
        return;
      }

      PropertyManager& pmgr = PropertyManager::GetInstance();

      std::string home_dir = ra::user::GetHomeDirectory();

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back(home_dir);
      c.SetElements(elements);

      c.RegisterProperties();

      //Execute the action as explained in https://stackoverflow.com/questions/38033790/shellexecute-a-cmd-prompt-with-a-specific-working-directory
      ActionExecute ae;
      ae.SetPath("cmd.exe");
      ae.SetBaseDir(home_dir);
      ae.SetArguments("/k cd /d ${selection.path}");
      ae.SetVerb("runas");

      bool executed = ae.Execute(c);
      ASSERT_TRUE(executed);
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testWaitInfinite)
    {
      ConfigManager& cmgr = ConfigManager::GetInstance();
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Creating a temporary workspace for the test execution.
      Workspace workspace;
      ASSERT_FALSE(workspace.GetBaseDirectory().empty());
      ASSERT_TRUE(workspace.IsEmpty());

      //Load the test Configuration File that matches this test name.
      QuickLoader loader;
      loader.SetWorkspace(&workspace);
      ASSERT_TRUE(loader.DeleteConfigurationFilesInWorkspace());
      ASSERT_TRUE(loader.LoadCurrentTestConfigurationFile());

      //Get first menu.
      ConfigFile::ConfigFilePtrList configs = cmgr.GetConfigFiles();
      ASSERT_EQ(1, configs.size());
      Menu::MenuPtrList menus = configs[0]->GetMenus();
      ASSERT_EQ(1, menus.size());
      Menu* menu = menus[0];

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows");
      c.SetElements(elements);
      c.RegisterProperties();

      // Execute
      uint64_t time_start = ra::timing::GetMillisecondsCounterU64();
      bool executed = ActionManager::Execute(menu, c);
      ASSERT_TRUE(executed);
      uint64_t time_end = ra::timing::GetMillisecondsCounterU64();
      uint64_t time_elapsed = time_end - time_start;

      //Assert elapsed time is around 10 seconds
      static const double time_expected = 10000.0;
      ASSERT_NEAR(time_expected, (double)time_elapsed, 2000.0);

      //Cleanup
      ASSERT_TRUE(workspace.Cleanup()) << "Failed deleting workspace directory '" << workspace.GetBaseDirectory() << "'.";
    }
    //--------------------------------------------------------------------------------------------------
    TEST_F(TestActionExecute, testWaitTimeout)
    {
      ConfigManager& cmgr = ConfigManager::GetInstance();
      PropertyManager& pmgr = PropertyManager::GetInstance();

      //Creating a temporary workspace for the test execution.
      Workspace workspace;
      ASSERT_FALSE(workspace.GetBaseDirectory().empty());
      ASSERT_TRUE(workspace.IsEmpty());

      //Load the test Configuration File that matches this test name.
      QuickLoader loader;
      loader.SetWorkspace(&workspace);
      ASSERT_TRUE(loader.DeleteConfigurationFilesInWorkspace());
      ASSERT_TRUE(loader.LoadCurrentTestConfigurationFile());

      //Get first menu.
      ConfigFile::ConfigFilePtrList configs = cmgr.GetConfigFiles();
      ASSERT_EQ(1, configs.size());
      Menu::MenuPtrList menus = configs[0]->GetMenus();
      ASSERT_EQ(1, menus.size());
      Menu* menu = menus[0];

      //Create a valid context
      SelectionContext c;
      StringList elements;
      elements.push_back("C:\\Windows");
      c.SetElements(elements);
      c.RegisterProperties();

      // Execute
      uint64_t time_start = ra::timing::GetMillisecondsCounterU64();
      bool executed = ActionManager::Execute(menu, c);
      //ASSERT_TRUE(executed); // the action should fail because of a timeout
      uint64_t time_end = ra::timing::GetMillisecondsCounterU64();
      uint64_t time_elapsed = time_end - time_start;

      //Assert elapsed time is around 10 seconds
      static const double time_expected = 10000.0;
      ASSERT_NEAR(time_expected, (double)time_elapsed, 2000.0);

      //Cleanup
      ASSERT_TRUE(workspace.Cleanup()) << "Failed deleting workspace directory '" << workspace.GetBaseDirectory() << "'.";
    }
    //--------------------------------------------------------------------------------------------------

  } //namespace test
} //namespace shellanything
