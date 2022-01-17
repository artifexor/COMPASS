/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <map>
#include <string>
#include <typeinfo>
#include <vector>

#include "configurableparameter.h"
#include "json.hpp"
#include "string.h"

/*
 *  @brief Configuration storage and retrieval container class
 *
 *  @details This class is used by a Configurable to store and retrieve parameters to/from an XML
 * configuration system.
 *
 *  Is held by a Configurable or can exist on its own.
 *
 *  \todo Extend registerParameter to template function.
 *  \todo Extend addParameter to template function.
 */
class Configuration
{
  public:
    /// @brief Constructor
    Configuration(const std::string& class_id, const std::string& instance_id,
                  const std::string& configuration_filename = "");

    /// @brief Copy constructor
    Configuration(const Configuration& source);
    /// @brief Destructor
    virtual ~Configuration();

    //    Configuration& operator= (const Configuration& source);
    //    Configuration* clone ();

    /// @brief Registers a boolean parameter
    void registerParameter(const std::string& parameter_id, bool* pointer, bool default_value);
    /// @brief Registers an int parameter
    void registerParameter(const std::string& parameter_id, int* pointer, int default_value);
    /// @brief Registers an unsigned int parameter
    void registerParameter(const std::string& parameter_id, unsigned int* pointer,
                           unsigned int default_value);
    /// @brief Registers a float parameter
    void registerParameter(const std::string& parameter_id, float* pointer, float default_value);
    /// @brief Registers a double parameter
    void registerParameter(const std::string& parameter_id, double* pointer, double default_value);
    /// @brief Registers a string parameter
    void registerParameter(const std::string& parameter_id, std::string* pointer,
                           const std::string& default_value);
    void registerParameter(const std::string& parameter_id, nlohmann::json* pointer,
                           const nlohmann::json& default_value);

    /// @brief Updates a boolean parameter pointer
    void updateParameterPointer(const std::string& parameter_id, bool* pointer);
    /// @brief Updates an int parameter pointer
    void updateParameterPointer(const std::string& parameter_id, int* pointer);
    /// @brief Updates an unsigned int parameter pointer
    void updateParameterPointer(const std::string& parameter_id, unsigned int* pointer);
    /// @brief Updates a float parameter pointer
    void updateParameterPointer(const std::string& parameter_id, float* pointer);
    /// @brief Updates a double parameter pointer
    void updateParameterPointer(const std::string& parameter_id, double* pointer);
    /// @brief Updates a string parameter pointer
    void updateParameterPointer(const std::string& parameter_id, std::string* pointer);
    void updateParameterPointer(const std::string& parameter_id, nlohmann::json* pointer);

    /// @brief Adds a boolean parameter
    void addParameterBool(const std::string& parameter_id, bool default_value);
    /// @brief Adds an integer parameter
    void addParameterInt(const std::string& parameter_id, int default_value);
    /// @brief Adds an unsigned int parameter
    void addParameterUnsignedInt(const std::string& parameter_id, unsigned int default_value);
    /// @brief Adds a float parameter
    void addParameterFloat(const std::string& parameter_id, float default_value);
    /// @brief Adds a double parameter
    void addParameterDouble(const std::string& parameter_id, double default_value);
    /// @brief Adds a string parameter
    void addParameterString(const std::string&, const std::string& default_value);
    void addParameterJSON(const std::string&, const nlohmann::json& default_value);

    /// @brief Writes data value if a boolean parameter to an argument
    void getParameter(const std::string& parameter_id, bool& value);
    /// @brief Writes data value if an integer parameter to an argument
    void getParameter(const std::string& parameter_id, int& value);
    /// @brief Writes data value if an unsigned int parameter to an argument
    void getParameter(const std::string& parameter_id, unsigned int& value);
    /// @brief Writes data value if a float parameter to an argument
    void getParameter(const std::string& parameter_id, float& value);
    /// @brief Writes data value if a double parameter to an argument
    void getParameter(const std::string& parameter_id, double& value);
    /// @brief Writes data value if a string parameter to an argument
    void getParameter(const std::string& parameter_id, std::string& value);
    void getParameter(const std::string& parameter_id, nlohmann::json& value);

    bool hasParameterConfigValueBool(const std::string& parameter_id);
    bool getParameterConfigValueBool(const std::string& parameter_id);
    bool hasParameterConfigValueInt(const std::string& parameter_id);
    int getParameterConfigValueInt(const std::string& parameter_id);
    bool hasParameterConfigValueUint(const std::string& parameter_id);
    unsigned int getParameterConfigValueUint(const std::string& parameter_id);
    bool hasParameterConfigValueFloat(const std::string& parameter_id);
    float getParameterConfigValueFloat(const std::string& parameter_id);
    bool hasParameterConfigValueDouble(const std::string& parameter_id);
    double getParameterConfigValueDouble(const std::string& parameter_id);
    bool hasParameterConfigValueString(const std::string& parameter_id);
    std::string getParameterConfigValueString(const std::string& parameter_id);
    nlohmann::json getParameterConfigValueJSON(const std::string& parameter_id);

    // parses the member config file
    void parseJSONConfigFile();
    void parseJSONConfig(nlohmann::json& config);
    // writes full json config or sub-file to parent
    void writeJSON(nlohmann::json& parent_json) const;
    // generates the full json config
    void generateJSON(nlohmann::json& target) const;

    /// @brief Resets all values to their default values
    void resetToDefault();

    /// @brief Creates added sub-configurables in configurable
    void createSubConfigurables(Configurable* configurable);

    /// @brief Returns flag indicating if configuration has been used by a configurable
    bool getUsed() { return used_; }

    /// @brief Sets special filename for XML configuration
    void setConfigurationFilename(const std::string& configuration_filename);
    /// @brief Returns flag if special filename has been set
    bool hasConfigurationFilename();
    /// @brief Return special filename
    const std::string& getConfigurationFilename();

    bool hasSubConfiguration(const std::string& class_id, const std::string& instance_id);
    /// @brief Adds a new sub-configuration and returns reference
    Configuration& addNewSubConfiguration(const std::string& class_id,
                                          const std::string& instance_id);
    /// @brief Adds a new sub-configuration and returns reference
    Configuration& addNewSubConfiguration(const std::string& class_id);
    /// @brief Returns a sub-configuration, creates new empty one if non-existing
    Configuration& addNewSubConfiguration(Configuration& configuration);
    /// @brief Returns a specfigied sub-configuration
    Configuration& getSubConfiguration(const std::string& class_id, const std::string& instance_id);
    /// @brief Removes a sub-configuration
    void removeSubConfiguration(const std::string& class_id, const std::string& instance_id);
    void removeSubConfigurations(const std::string& class_id);

    /// @brief Returns the instance identifier
    const std::string& getInstanceId() { return instance_id_; }
    /// @brief Returns the class identifier
    const std::string& getClassId() { return class_id_; }

    /// @brief Sets the template flag and name
    // void setTemplate (bool template_flag, const std::string& template_name_);
    /// @brief Returns the template flag
    // bool getTemplateFlag () const { return template_flag_; }
    /// @brief Returns the template name
    // const std::string& getTemplateName () { return template_name_; }

    /// @brief Checks if a specific template_name is already taken, true if free
    bool getSubTemplateNameFree(const std::string& template_name);
    /// @brief Adds a template configuration with a name
    void addSubTemplate(Configuration* configuration, const std::string& template_name);

    /// @brief Return contaienr with all configuration templates
    // std::map<std::string, Configuration>& getConfigurationTemplates () { return
    // configuration_templates_; }

    // only use in special case of configuration copy
    void setInstanceId(const std::string& instance_id) { instance_id_ = instance_id; }

  protected:
    /// Class identifier
    std::string class_id_;
    /// Instance identifier
    std::string instance_id_;
    /// Flag indicating if configuration has been used by configurable
    bool used_{false};
    /// Special XML configuration filename
    std::string configuration_filename_;

    nlohmann::json org_config_parameters_;
    // nlohmann::json org_config_sub_files_;
    // nlohmann::json org_config_sub_configs_;

    /// Container for all parameters (parameter identifier -> ConfigurableParameterBase)
    std::map<std::string, ConfigurableParameter<bool> > parameters_bool_;
    std::map<std::string, ConfigurableParameter<int> > parameters_int_;
    std::map<std::string, ConfigurableParameter<unsigned int> > parameters_uint_;
    std::map<std::string, ConfigurableParameter<float> > parameters_float_;
    std::map<std::string, ConfigurableParameter<double> > parameters_double_;
    std::map<std::string, ConfigurableParameter<std::string> > parameters_string_;
    std::map<std::string, ConfigurableParameter<nlohmann::json> > parameters_json_;
    /// Container for all added sub-configurables
    std::map<std::pair<std::string, std::string>, Configuration> sub_configurations_;

    /// Flag which indicates if instance is a template
    // bool template_flag_ {false};
    /// Template name, empty if no template
    // std::string template_name_;

    /// Container with all configuration templates
    // std::map<std::string, Configuration> configuration_templates_;

    void parseJSONSubConfigFile(const std::string& class_id, const std::string& instance_id,
                                const std::string& path);
    void parseJSONParameters(nlohmann::json& parameters_config);
    void parseJSONSubConfigs(nlohmann::json& sub_configs_config);
};

#endif /* CONFIGURATION_H_ */
