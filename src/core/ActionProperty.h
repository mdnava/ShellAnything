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

#ifndef SA_ACTION_PROPERTY_H
#define SA_ACTION_PROPERTY_H

#include "IAction.h"
#include "BaseAction.h"
#include "IActionFactory.h"

namespace shellanything
{

  /// <summary>
  /// Action class that changes the value of a property.
  /// </summary>
  class SHELLANYTHING_EXPORT ActionProperty : public BaseAction
  {
  public:
    ActionProperty();
    virtual ~ActionProperty();

    /// <summary>
    /// Default maximum number of bytes for reading a property from a file.
    /// </summary>
    static const size_t DEFAULT_MAX_FILE_SIZE;

    /// <summary>
    /// Default value for 'fail' attribute.
    /// </summary>
    static const std::string DEFAULT_FAIL_VALUE;

    /// <summary>
    /// Name of the xml element for this action.
    /// </summary>
    static const std::string XML_ELEMENT_NAME;

    /// <summary>
    /// Instanciate an IActionFactory that is able to parse this action.
    /// </summary>
    /// <returns>Returns a IActionFactory to parse this action.</returns>
    static IActionFactory* NewFactory();

    /// <summary>
    /// Changes the value of a property.
    /// </summary>
    /// <param name="context">The current context of execution.</param>
    /// <returns>Returns true if the execution is successful. Returns false otherwise.</returns>
    virtual bool Execute(const SelectionContext& context) const;

    /// <summary>
    /// Getter for the 'name' parameter.
    /// </summary>
    const std::string& GetName() const;

    /// <summary>
    /// Setter for the 'name' parameter.
    /// </summary>
    void SetName(const std::string& name);

    /// <summary>
    /// Getter for the 'value' parameter.
    /// </summary>
    const std::string& GetValue() const;

    /// <summary>
    /// Setter for the 'value' parameter.
    /// </summary>
    void SetValue(const std::string& value);

    /// <summary>
    /// Getter for the 'exprtk' parameter.
    /// </summary>
    const std::string& GetExprtk() const;

    /// <summary>
    /// Setter for the 'exprtk' parameter.
    /// </summary>
    void SetExprtk(const std::string& exprtk);

    /// <summary>
    /// Getter for the 'file' parameter.
    /// </summary>
    const std::string& GetFile() const;

    /// <summary>
    /// Setter for the 'file' parameter.
    /// </summary>
    void SetFile(const std::string& value);

    /// <summary>
    /// Getter for the 'filesize' parameter.
    /// </summary>
    const std::string& GetFileSize() const;

    /// <summary>
    /// Setter for the 'filesize' parameter.
    /// </summary>
    void SetFileSize(const std::string& value);

    /// <summary>
    /// Getter for the 'registrykey' parameter.
    /// </summary>
    const std::string& GetRegistryKey() const;

    /// <summary>
    /// Setter for the 'registrykey' parameter.
    /// </summary>
    void SetRegistryKey(const std::string& value);

    /// <summary>
    /// Getter for the 'searchpath' parameter.
    /// </summary>
    const std::string& GetSearchPath() const;

    /// <summary>
    /// Setter for the 'searchpath' parameter.
    /// </summary>
    void SetSearchPath(const std::string& value);

    /// <summary>
    /// Getter for the 'random' parameter.
    /// </summary>
    const std::string& GetRandom() const;

    /// <summary>
    /// Setter for the 'random' parameter.
    /// </summary>
    void SetRandom(const std::string& value);

    /// <summary>
    /// Getter for the 'randommin' parameter.
    /// </summary>
    const std::string& GetRandomMin() const;

    /// <summary>
    /// Setter for the 'randommin' parameter.
    /// </summary>
    void SetRandomMin(const std::string& value);

    /// <summary>
    /// Getter for the 'randommax' parameter.
    /// </summary>
    const std::string& GetRandomMax() const;

    /// <summary>
    /// Setter for the 'randommax' parameter.
    /// </summary>
    void SetRandomMax(const std::string& value);

    /// <summary>
    /// Getter for the 'fail' parameter.
    /// </summary>
    const std::string& GetFail() const;

    /// <summary>
    /// Setter for the 'fail' parameter.
    /// </summary>
    void SetFail(const std::string& value);

  private:
    bool GetValueFromExprtk(const std::string& exprtk, std::string& value) const;
    bool GetValueFromFile(const std::string& file, const std::string& filesize, std::string& value) const;
    bool GetValueFromRegistryKey(const std::string& registrykey, std::string& value) const;
    bool GetValueFromSearchPath(const std::string& searchpath, std::string& value) const;
    bool GetValueFromRandom(const std::string& random, const std::string& random_min, std::string& random_max, std::string& value) const;

    std::string mName;
    std::string mValue;
    std::string mFail;
    std::string mExprtk;
    std::string mFile;
    std::string mFileSize;
    std::string mRegistryKey;
    std::string mSearchPath;
    std::string mRandom;
    std::string mRandomMin;
    std::string mRandomMax;
  };


} //namespace shellanything

#endif //SA_ACTION_PROPERTY_H
