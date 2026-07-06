#include "TGTIPlugin.h"
#include <arch/MemoryOut.h>
#include <fo/FileOperations.h>
#include <mu/ScopedCLocale.h>
#include <td/MutableString.h>
#include <td/StringUtils.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>

static void tgtiCoreTrace(const char* message)
{
    std::ofstream f("C:\\Users\\User\\Documents\\SREES TGTI 2\\TGTI\\tgti_plugin_trace.log", std::ios::app);
    if (f)
        f << message << "\n";
}

namespace
{
struct MatpowerCase
{
    double baseMVA = 100.0;
    std::vector<std::vector<double>> genRows;
};

static std::string loadTextFile(const td::String& fileName)
{
    std::ifstream in(fileName.c_str(), std::ios::binary);
    if (!in)
        return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static std::string getAttr(const std::string& text, const char* tag, const char* attr)
{
    std::string tagName(tag);
    std::string attrName(attr);
    auto tagPos = text.find("<" + tagName);
    if (tagPos == std::string::npos)
        return {};
    auto tagEnd = text.find(">", tagPos);
    if (tagEnd == std::string::npos)
        return {};
    std::string tagText = text.substr(tagPos, tagEnd - tagPos);
    auto attrPos = tagText.find(attrName + "=\"");
    if (attrPos == std::string::npos)
        return {};
    attrPos += attrName.size() + 2;
    auto attrEnd = tagText.find("\"", attrPos);
    if (attrEnd == std::string::npos)
        return {};
    return tagText.substr(attrPos, attrEnd - attrPos);
}

static void applyXmlConfig(const td::String& fileName, TGTIOptions& options)
{
    if (fileName.isEmpty())
        return;
    std::string xml = loadTextFile(fileName);
    if (xml.empty())
        return;
    
    auto generatorNo = getAttr(xml, "Generator", "number");
    if (!generatorNo.empty())
        options.generatorNo = td::INT4(std::atoi(generatorNo.c_str()));
    
    auto maxIter = getAttr(xml, "Simulation", "maxIter");
    auto dTime = getAttr(xml, "Simulation", "dTime");
    auto endTime = getAttr(xml, "Simulation", "endTime");
    if (!maxIter.empty())
        options.maxIter = td::INT4(std::atoi(maxIter.c_str()));
    if (!dTime.empty())
        options.dTime = float(std::atof(dTime.c_str()));
    if (!endTime.empty())
        options.endTime = float(std::atof(endTime.c_str()));
    
    auto R = getAttr(xml, "Parameters", "R");
    auto Ts = getAttr(xml, "Parameters", "Ts");
    auto Tc = getAttr(xml, "Parameters", "Tc");
    auto T3 = getAttr(xml, "Parameters", "T3");
    auto T4 = getAttr(xml, "Parameters", "T4");
    auto T5 = getAttr(xml, "Parameters", "T5");
    auto pmax = getAttr(xml, "Parameters", "pmax");
    auto pmin = getAttr(xml, "Parameters", "pmin");
    if (!R.empty()) options.params.R = std::atof(R.c_str());
    if (!Ts.empty()) options.params.Ts = std::atof(Ts.c_str());
    if (!Tc.empty()) options.params.Tc = std::atof(Tc.c_str());
    if (!T3.empty()) options.params.T3 = std::atof(T3.c_str());
    if (!T4.empty()) options.params.T4 = std::atof(T4.c_str());
    if (!T5.empty()) options.params.T5 = std::atof(T5.c_str());
    if (!pmax.empty()) options.params.pmax = std::atof(pmax.c_str());
    if (!pmin.empty()) options.params.pmin = std::atof(pmin.c_str());
}

static std::string stripMatlabComment(const std::string& line)
{
    auto pos = line.find('%');
    if (pos == std::string::npos)
        return line;
    return line.substr(0, pos);
}

static bool parseBaseMVA(const std::string& line, double& baseMVA)
{
    auto pos = line.find("mpc.baseMVA");
    if (pos == std::string::npos)
        return false;
    auto eq = line.find('=', pos);
    if (eq == std::string::npos)
        return false;
    baseMVA = std::atof(line.c_str() + eq + 1);
    return true;
}

static bool parseMatrixRow(const std::string& line, std::vector<double>& row)
{
    std::string clean = stripMatlabComment(line);
    for (char& ch: clean)
    {
        if (ch == '[' || ch == ']' || ch == ';' || ch == ',')
            ch = ' ';
    }
    std::istringstream ss(clean);
    double val = 0.0;
    while (ss >> val)
        row.push_back(val);
    return !row.empty();
}

static bool loadMatpowerCase(const td::String& fileName, MatpowerCase& mpCase, td::String& status)
{
    std::ifstream in(fileName.c_str());
    if (!in)
    {
        status = "ERROR! Cannot open MATPOWER case.";
        return false;
    }
    
    std::string line;
    bool inGen = false;
    while (std::getline(in, line))
    {
        parseBaseMVA(line, mpCase.baseMVA);
        if (!inGen && line.find("mpc.gen") != std::string::npos && line.find('[') != std::string::npos)
        {
            inGen = true;
            auto bracket = line.find('[');
            line = line.substr(bracket + 1);
        }
        
        if (inGen)
        {
            bool endGen = line.find("];") != std::string::npos || line.find(']') != std::string::npos;
            std::vector<double> row;
            if (parseMatrixRow(line, row))
                mpCase.genRows.push_back(row);
            if (endGen)
                inGen = false;
        }
    }
    
    if (mpCase.genRows.empty())
    {
        status = "ERROR! MATPOWER mpc.gen matrix was not found.";
        return false;
    }
    
    size_t cols = mpCase.genRows[0].size();
    if (cols < 2)
    {
        status = "ERROR! Generator matrix must have at least 2 columns.";
        return false;
    }
    
    for (size_t i = 0; i < mpCase.genRows.size(); ++i)
    {
        if (mpCase.genRows[i].size() != cols)
        {
            status = "ERROR! Inconsistent generator row length in MATPOWER case.";
            return false;
        }
    }
    
    return true;
}

static void put(arch::MemoryOut* pOut, const td::String& text)
{
    if (pOut)
        pOut->put(text.c_str(), text.length());
}

static void put(arch::MemoryOut* pOut, const char* text)
{
    if (pOut)
        pOut->put(text);
}

static void appendTGTIModel(td::MutableString& out, const TGTIOptions& options, const MatpowerCase& mpCase)
{
    const TGTIParameters& p = options.params;
    td::String modelName = options.modelName;
    if (modelName.isEmpty())
        modelName = "Turbine Governor Type I";
    modelName.replace('\"', '\'');
    
    td::UINT4 nGen = td::UINT4(mpCase.genRows.size());
    td::INT4 selected = options.generatorNo;
    if (selected < 1)
        selected = 1;
    if (selected > td::INT4(nGen))
        selected = td::INT4(nGen);

    const double pOrder = mpCase.genRows[size_t(selected - 1)][1] / mpCase.baseMVA;
    
    out.appendFormat("Header:\n\tmaxIter = %d\n\treport = Solved\n\tmaxReps = -1\n\toutToTxt = false\n\ttxtFile = \"\"\n\tstartTime = 0\n\tdTime = %.6f\n\tendTime = %.6f\nend\n\n",
                     options.maxIter, double(options.dTime), double(options.endTime));
    out.append("// Model created by TGTI natID/dTwin plugin converter\n");
    out.appendFormat("// MATPOWER generators: %u, selected generator: %d\n\n", nGen, selected);
    out.appendFormat("Model [type=DAE domain=real method=RK4 name=\"%s\"]:\n", modelName.c_str());
    
    out.append("Vars [out=true]:\n");
    out.appendFormat("\tu_g = %.10g\n", pOrder);
    out.appendFormat("\tx_g = %.10g\n", pOrder);
    out.appendFormat("\tp_in = %.10g\n", pOrder);
    out.appendFormat("\tx_c = %.10g\n", pOrder);
    out.appendFormat("\ttau_m = %.10g\n", pOrder);
    if (options.useStandardForOthers)
    {
        for (td::UINT4 i = 1; i <= nGen; ++i)
        {
            if (td::INT4(i) == selected)
                continue;
            const double pg = mpCase.genRows[size_t(i - 1)][1] / mpCase.baseMVA;
            out.appendFormat("\ttau_m_%u = %.10g\n", i, pg);
        }
    }
    
    out.append("\nParams:\n");
    out.appendFormat("\tp_order = %.10g [out=true]\n", pOrder);
    out.append("\tomega_ref = 1.00\n");
    out.append("\tomega = 0.99 [out=true]\n");
    out.appendFormat("\tR = %.10g [out=true]\n", p.R);
    out.appendFormat("\tTs = %.10g [out=true]\n", p.Ts);
    out.appendFormat("\tTc = %.10g [out=true]\n", p.Tc);
    out.appendFormat("\tT3 = %.10g [out=true]\n", p.T3);
    out.appendFormat("\tT4 = %.10g [out=true]\n", p.T4);
    out.appendFormat("\tT5 = %.10g [out=true]\n", p.T5);
    out.appendFormat("\tpmax = %.10g [out=true]\n", p.pmax);
    out.appendFormat("\tpmin = %.10g [out=true]\n", p.pmin);
    if (options.useStandardForOthers)
    {
        for (td::UINT4 i = 1; i <= nGen; ++i)
        {
            if (td::INT4(i) == selected)
                continue;
            const double pg = mpCase.genRows[size_t(i - 1)][1] / mpCase.baseMVA;
            out.appendFormat("\tp_order_%u = %.10g [out=true]\n", i, pg);
        }
    }
    
    out.append("\nTFs:\n");
    out.append("\tx_g/p_in = 1/(Ts*s + 1)\n");
    out.append("\tx_c/x_g = (T3*s + 1)/(Tc*s + 1)\n");
    out.append("\ttau_m/x_c = (T4*s + 1)/(T5*s + 1)\n");
    
    out.append("\nNLEs:\n");
    out.append("\tu_g = p_order + (omega_ref - omega)/R\n");
    out.append("\tp_in = lim(x_g, pmin, pmax)\n");
    if (options.useStandardForOthers)
    {
        for (td::UINT4 i = 1; i <= nGen; ++i)
        {
            if (td::INT4(i) == selected)
                continue;
            out.appendFormat("\ttau_m_%u = p_order_%u\n", i, i);
        }
    }
    out.append("end\n");
}

static bool writeOutputFile(const td::String& fileName, const td::MutableString& content)
{
    std::ofstream out(fileName.c_str(), std::ios::binary);
    if (!out)
        return false;
    out.write(content.c_str(), std::streamsize(content.length()));
    return bool(out);
}

static void appendTGTIVisualModel(td::MutableString& out)
{
    out.append("Header:\n");
    out.append("end  //end of header\n");
    out.append("//Show TGTI plots\n");
    out.append("Model:\n");
    out.append("Plots:\n");
    out.append("\tlinePlot [xLabel=\"Time [s]\" yLabel=\"p_in,x_g,x_c,tau_m\" name=\"TGTI response\" anchor=Top legend=true nCols=1 anchorX=-15 anchorY=32]:\n");
    out.append("\t\t@x << t\n");
    out.append("\t\t@y << p_in [colorL=black colorD=red width=2 name=\"p_in\"]\n");
    out.append("\t\t@y << x_g [colorL=darkGreen colorD=green width=2 name=\"x_g\"]\n");
    out.append("\t\t@y << x_c [width=2 pattern=\"dash\" name=\"x_c\"]\n");
    out.append("\t\t@y << tau_m [width=2 pattern=\"dot\" name=\"tau_m\"]\n");
    out.append("\t\t@cond -> repeat# == 0\n");
    out.append("\tend\n");
    out.append("\t\t\n");
    out.append("end //end of visual model\n");
}

static std::string makeVisualModelFileName(const td::String& outFileName)
{
    std::string fileName(outFileName.c_str());
    const size_t slashPos = fileName.find_last_of("\\/");
    const size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos))
        fileName.replace(dotPos, std::string::npos, ".vmodl");
    else
        fileName += ".vmodl";
    return fileName;
}

static bool writeTextFile(const std::string& fileName, const td::MutableString& content)
{
    std::ofstream out(fileName.c_str(), std::ios::binary);
    if (!out)
        return false;
    out.write(content.c_str(), std::streamsize(content.length()));
    return bool(out);
}
}

bool createTGTIModel(const td::String& inputFileName,
                     const td::String& outFileName,
                     sc::IPlugin* pIPlugin,
                     const TGTIOptions& inputOptions,
                     ConversionProgress& progress,
                     td::String& status)
{
    tgtiCoreTrace("createTGTIModel entered");
    mu::ScopedCLocale scopedLocale;
    TGTIOptions options = inputOptions;
    
    progress.percent = 5;
    progress.setMessage("Reading XML configuration...");
    tgtiCoreTrace("before applyXmlConfig");
    applyXmlConfig(options.configFileName, options);
    
    progress.percent = 25;
    progress.setMessage("Reading MATPOWER case...");
    tgtiCoreTrace("before loadMatpowerCase");
    MatpowerCase mpCase;
    if (!loadMatpowerCase(inputFileName, mpCase, status))
    {
        tgtiCoreTrace("loadMatpowerCase failed");
        progress.failed = true;
        return false;
    }
    
    progress.percent = 55;
    progress.setMessage("Generating dTwin DAE model...");
    tgtiCoreTrace("before appendTGTIModel");
    td::MutableString modelText;
    modelText.reserve(16 * 1024);
    appendTGTIModel(modelText, options, mpCase);
    
    progress.percent = 75;
    progress.setMessage("Writing plugin archives...");
    tgtiCoreTrace(pIPlugin ? "before plugin archive write" : "plugin archive write skipped");
    auto pDigitalModel = pIPlugin ? pIPlugin->getArchive(sc::IPlugin::ArchType::DigitalModel) : nullptr;
    put(pDigitalModel, modelText.c_str());
    
    auto pVisualModel = pIPlugin ? pIPlugin->getArchive(sc::IPlugin::ArchType::VisualModel) : nullptr;
    put(pVisualModel, "// TGTI visual model placeholder\n");
    put(pVisualModel, "// Use OpenDraw diagrams in docs and replace this section when dTwin visual model format is available.\n");
    
    progress.percent = 90;
    progress.setMessage("Writing output file...");
    tgtiCoreTrace("before writeOutputFile");
    if (!writeOutputFile(outFileName, modelText))
    {
        tgtiCoreTrace("writeOutputFile failed");
        status = "ERROR! Cannot write output .dmodl file.";
        progress.failed = true;
        return false;
    }

    td::MutableString visualText;
    visualText.reserve(4 * 1024);
    appendTGTIVisualModel(visualText);
    const std::string visualFileName = makeVisualModelFileName(outFileName);
    if (!writeTextFile(visualFileName, visualText))
        tgtiCoreTrace("write vmodl failed");

    progress.percent = 100;
    status = "Conversion completed. Matching .vmodl plot file was created.";
    progress.setMessage(status);
    tgtiCoreTrace("createTGTIModel completed");
    return true;
}
