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

#include "rtcommand_string.h"
#include "rtcommand_registry.h"

#include "logger.h"

#include <iostream>

#include <boost/program_options.hpp>

#include <QStringList>
#include <QRegularExpression>
#include <QProcess>

namespace rtcommand
{

namespace 
{
    QStringList splitCommand(const QString& command)
    {
        //return QProcess::splitCommand(cmd); // does only exist in Qt 5.15+

        QStringList args;
        QString tmp;
        int quoteCount = 0;
        bool inQuote = false;
        // handle quoting. tokens can be surrounded by double quotes
        // "hello world". three consecutive double quotes represent
        // the quote character itself.
        for (int i = 0; i < command.size(); ++i) {
            if (command.at(i) == QLatin1Char('"')) {
                ++quoteCount;
                if (quoteCount == 3) {
                    // third consecutive quote
                    quoteCount = 0;
                    tmp += command.at(i);
                }
                continue;
            }
            if (quoteCount) {
                if (quoteCount == 1)
                    inQuote = !inQuote;
                quoteCount = 0;
            }
            if (!inQuote && command.at(i).isSpace()) {
                if (!tmp.isEmpty()) {
                    args += tmp;
                    tmp.clear();
                }
            } else {
                tmp += command.at(i);
            }
        }
        if (!tmp.isEmpty())
            args += tmp;
        return args;
    }
}

/**
*/
RTCommandString::RTCommandString(const QString& cmd)
:   cmd_(cmd.trimmed())
{
    cmd_name_ = extractName();
}

/**
*/
RTCommandString::~RTCommandString() = default;

/**
*/
QString RTCommandString::extractName() const
{
    if (cmd_.isEmpty())
        return "";
    
    auto s = splitCommand(cmd_);
    if (s.isEmpty())
        return "";

    return s[ 0 ];
}

/**
*/
QString RTCommandString::cmdName() const
{
    return cmd_name_;
}

/**
*/
QString RTCommandString::cmd() const
{ 
    return cmd_;
}

/**
*/
bool RTCommandString::valid() const
{
    return (!cmd_name_.isEmpty());
}

/**
*/
QString RTCommandString::quoteString(const QString& s) const
{ 
    return "\"" + s + "\"";
}

/**
*/
QString RTCommandString::paramFull(const QString& name, const QString& value, bool quote) const
{ 
    return "--" + name + "=" + (quote ? quoteString(value) : value); 
}

/**
*/
QString RTCommandString::paramShort(const QString& name, const QString& value, bool quote) const
{ 
    return "-" + name + " " + (quote ? quoteString(value) : value); 
}

/**
*/
RTCommandString& RTCommandString::append(const QString& name, 
                                         const QString& value, 
                                         bool is_short, 
                                         bool quote)
{  
    cmd_ += " ";
    cmd_ += (is_short ? paramShort(name, value, quote) : 
                        paramFull (name, value, quote));
    return (*this);
}

/**
*/
bool RTCommandString::hasHelpOption() const
{
    if (cmd_.isEmpty() || !valid())
        return false;

    QStringList parts = splitCommand(cmd_);
    int argc = parts.count();

    if (argc < 1)
        return false;

    for (int i = 1; i < argc; ++i)
    {
        QString s = parts[ i ].trimmed();
        if (s == "-h" || s == "--help")
            return true;
    }

    return false;
}

/**
*/
bool RTCommandString::parse(boost::program_options::variables_map& vm, 
                            const boost::program_options::options_description& d,
                            bool drop_quotes) const
{
    namespace po = boost::program_options;

    if (cmd_.isEmpty() || !valid())
        return false;

    QStringList parts = splitCommand(cmd_);
    int argc = parts.count();

    if (argc < 1)
        return false;

    std::vector<std::string> strings(argc);
    std::vector<const char*> argv   (argc);
    for (int i = 0; i < argc; ++i)
    {
        strings[ i ] = parts  [ i ].toStdString();
        argv   [ i ] = strings[ i ].c_str();
    }
    
    try
    {
        po::store(po::parse_command_line(argc, argv.data(), d), vm);
        po::notify(vm);
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error parsing command: " << ex.what() << std::endl;
        return false;
    }

    if (drop_quotes)
    {
        //chop quotes from all variable strings
        for (auto& elem : vm)
        {
            try
            {
                //try to cast var to string
                auto& str = elem.second.as<std::string>();

                QString s = QString::fromStdString(str);
                if (s.startsWith("\"") && 
                    s.endsWith("\"") && 
                    s.count() >= 2)
                {
                    s = s.mid(1, s.count() - 2);
                    str = s.toStdString();
                }
            }
            catch (...)
            {
                continue;
            }
        }
    }

    return true;
}

/**
*/
std::unique_ptr<RTCommand> RTCommandString::issue() const
{
    if (!valid())
    {
        logerr << "RTCommandString::issue(): Command string not valid";
        return nullptr;
    }

    const QString cmd_str  = cmd();
    const QString cmd_name = cmdName();

    loginf << "Command to process: " << cmd_str.toStdString();
    loginf << "Creating command template from name '" << cmd_name.toStdString() << "'";

    if (!rtcommand::RTCommandRegistry::instance().hasCommand(cmd_name))
    {
        logerr << "RTCommandString::issue(): Command not registered";
        return nullptr;
    }

    //preparse command for help option
    if (hasHelpOption())
    {
        //help option detected, return help command
        loginf << "Help option detected...";

        auto cmd_help = new RTCommandHelp;
        cmd_help->command = cmd_name;
        
        return std::unique_ptr<RTCommand>(cmd_help);
    }

    auto cmdObj = rtcommand::RTCommandRegistry::instance().createCommandTemplate(cmd_name);
    if (!cmdObj)
    {
        logerr << "RTCommandString::issue(): Registry returned nullptr";
        return nullptr;
    }

    loginf << "Configuring command template...";

    if (!cmdObj->configure(*this))
    {
        loginf << "RTCommandString::issue(): Command could not be configured";
        return nullptr;
    }

    return cmdObj;
}

} // namespace rtcommand
