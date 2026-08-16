#include "Generater.h"
#include "Parser.h"
#include <filesystem>
#include <fstream>
#include <sstream>
static const std::string generatedExtension = ".generated";


inline static std::string BuildAttributeList(const std::vector<AttributeInfo>& attributes)
{
	std::stringstream out;
	for (size_t i = 0; i < attributes.size(); i++)
	{
		const auto& attr = attributes[i];
		out << "ATTR(" << attr.name;
		for (const auto& arg : attr.args)
		{
			out << ", \"" << arg << "\"";
		}
		out << ")";
		if (i < attributes.size() - 1)
		{
			out << ", ";
		}
	}
	return out.str();
}

Generater::Generater(const std::string& outputDir)
	: outputDirectory(outputDir)
{
	if (!std::filesystem::exists(outputDirectory))
	{
		std::filesystem::create_directories(outputDirectory);
	}
}

void Generater::Generate(const std::vector<FileInfo>& files, const std::vector<std::string>& ignoreClasses)
{
	knownEnums = BuildKnownEnums(files);

	// 各ファイルごとに処理
	for (const auto& info : files)
	{
		// 元ヘッダーファイルの相対パス
		std::string relativePathFromHeader = std::filesystem::relative(info.path, outputDirectory + "/" + headerDir).string();
		std::string relativePathFromSource = std::filesystem::relative(info.path, outputDirectory + "/" + sourceDir).string();
		std::string relativePathFromEnum = std::filesystem::relative(info.path, outputDirectory + "/" + enumDir).string();
		std::string relativePathFromStruct = std::filesystem::relative(info.path, outputDirectory + "/" + structDir).string();
		// 変換してスラッシュ区切りに
		std::replace(relativePathFromHeader.begin(), relativePathFromHeader.end(), '\\', '/');
		std::replace(relativePathFromSource.begin(), relativePathFromSource.end(), '\\', '/');
		std::replace(relativePathFromEnum.begin(), relativePathFromEnum.end(), '\\', '/');
		std::replace(relativePathFromStruct.begin(), relativePathFromStruct.end(), '\\', '/');

		// 生成ソースからソリューションルートへの相対パス (例: "../../")
		//std::string relativeSolutionPath = std::filesystem::relative(std::filesystem::current_path(), std::filesystem::absolute(relativePathFromSource)).string();
		//std::replace(relativeSolutionPath.begin(), relativeSolutionPath.end(), '\\', '/');

		// 出力ディレクトリからの相対パスを計算
		for (const auto& classInfo : info.classes)
		{
			if (!classInfo.reflect) continue;
			std::filesystem::path headerOutputDir = std::filesystem::path(outputDirectory) / headerDir;
			std::filesystem::path sourceOutputDir = std::filesystem::path(outputDirectory) / sourceDir;
			if (!std::filesystem::exists(headerOutputDir))
			{
				std::filesystem::create_directories(headerOutputDir);
			}
			if (!std::filesystem::exists(sourceOutputDir))
			{
				std::filesystem::create_directories(sourceOutputDir);
			}
			std::string headerPath = (headerOutputDir / std::filesystem::path(classInfo.name + generatedExtension + ".h")).string();
			std::string sourcePath = (sourceOutputDir / std::filesystem::path(classInfo.name + generatedExtension + ".cpp")).string();
			
			std::filesystem::path relativeSolutionPath = std::filesystem::relative(std::filesystem::current_path(), std::filesystem::absolute(sourcePath)).parent_path();
			std::string relativeSolutionPathStr = relativeSolutionPath.string();
			std::replace(relativeSolutionPathStr.begin(), relativeSolutionPathStr.end(), '\\', '/');

			GenerateHeader(classInfo, headerPath, relativePathFromHeader);
			// 一部のクラスは除外する(GameObjectなどのコンポーネントを継承しないクラスは生成しない)
			bool allowSourceGen = true;
			for (const auto& ignore : ignoreClasses)
			{
				if (classInfo.name == ignore)
				{
					allowSourceGen = false;
					break;
				}
			}
			if (allowSourceGen)
			{
				GenerateSource(classInfo, sourcePath, relativePathFromSource, relativeSolutionPathStr);
			}
		}

		// enum / enum class の生成
		for (const auto& enumInfo : info.enums)
		{
			std::filesystem::path headerOutputDir = std::filesystem::path(outputDirectory) / enumDir;
			if (!std::filesystem::exists(headerOutputDir))
			{
				std::filesystem::create_directories(headerOutputDir);
			}
			std::string headerPath = (headerOutputDir / std::filesystem::path(enumInfo.name + generatedExtension + ".h")).string();
			GenerateEnum(enumInfo, headerPath, relativePathFromEnum);
		}

		// struct の生成
		for (const auto& structInfo : info.structs)
		{
			std::filesystem::path headerOutputDir = std::filesystem::path(outputDirectory) / structDir;
			if (!std::filesystem::exists(headerOutputDir))
			{
				std::filesystem::create_directories(headerOutputDir);
			}
			std::string headerPath = (headerOutputDir / std::filesystem::path(structInfo.name + generatedExtension + ".h")).string();
			GenerateStruct(structInfo, headerPath, relativePathFromStruct);
		}
	}

	// ReflectionGenerated.h を生成
	{
		std::ofstream ofs(outputDirectory + "/ReflectionGenerated.h");
		ofs << "#pragma once\n\n";

		for (const auto& info : files)
		{
			ofs << "// Original header: " << info.path << "\n";
			ofs << "\n// Classes\n";
			for (const auto& classInfo : info.classes)
			{
				if (!classInfo.reflect) continue;
				ofs << "#include \"" << headerDir << "/" << classInfo.name << generatedExtension + ".h\"\n";
			}
			ofs << "\n// Enums\n";
			for (const auto& enumInfo : info.enums)
			{
				ofs << "#include \"" << enumDir << "/" << enumInfo.name << generatedExtension + ".h\"\n";
			}
			ofs << "\n// Structs\n";
			for (const auto& structInfo : info.structs)
			{
				ofs << "#include \"" << structDir << "/" << structInfo.name << generatedExtension + ".h\"\n";
			}
			ofs << "\n";
		}
	}
}

void Generater::GenerateHeader(const ClassInfo& info, const std::string& outPath, const std::string& includePath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}
	ofs << "#pragma once\n\n";
	ofs << "#include \"" << includePath << "\"\n";
	ofs << "#include \"" << "Engine/Core/Reflection/Meta.h" << "\"\n";

	ofs << "REGISTER_CLASS(" << info.name << ", ";
	for (size_t i = 0; i < info.bases.size(); i++)
	{
		ofs << info.bases[i];
		if (i < info.bases.size() - 1)
			ofs << ", ";
	}
	ofs << ")\n";
	for (const auto& field : info.fields)
	{
		// customGetter/customSetter の有無で4パターンに分岐
		bool hasGetter = !field.customGetter.empty();
		bool hasSetter = !field.customSetter.empty();
		bool hasAttributes = !field.attributes.empty();

		if (!hasGetter && !hasSetter)
		{
			if (!hasAttributes)
			{
				// 属性がない場合は通常のマクロを使用
				ofs << "    REGISTER_PROPERTY("
					<< info.name << ", "
					<< field.name << ", "
					<< field.type << ")\n";
			}
			else
			{
				// 属性がある場合は属性付きマクロを使用
				ofs << "    REGISTER_PROPERTY_WITH_ATTR("
					<< info.name << ", "
					<< field.name << ", "
					<< field.type << ", "
					<< BuildAttributeList(field.attributes) << ")\n";
			}
		}
		else if (hasGetter && !hasSetter)
		{
			// Getter のみ指定されている場合はセッターファンクションに空文字列を渡す
			ofs << "    REGISTER_PROPERTY_WITH_CUSTOM_GETTER("
				<< info.name << ", "
				<< field.name << ", "
				<< field.type << ", "
				<< field.customGetter;
			// 属性リストを出力
			if (hasAttributes)
			{
				ofs << ", " << BuildAttributeList(field.attributes);
			}
			ofs << ")\n";
		}
		else if (!hasGetter && hasSetter)
		{
			// Setter のみ指定されている場合はゲッターファンクションに空文字列を渡す
			ofs << "    REGISTER_PROPERTY_WITH_CUSTOM_SETTER("
				<< info.name << ", "
				<< field.name << ", "
				<< field.type << ", "
				<< field.customSetter;
			// 属性リストを出力
			if (hasAttributes)
			{
				ofs << ", " << BuildAttributeList(field.attributes);
			}
			ofs << ")\n";
		}
		else
		{
			// Getter と Setter の両方が指定されている場合は両方を渡す
			ofs << "    REGISTER_PROPERTY_WITH_CUSTOM_ACCESSOR("
				<< info.name << ", "
				<< field.name << ", "
				<< field.type << ", "
				<< field.customGetter << ", "
				<< field.customSetter;
			// 属性リストを出力
			if (hasAttributes)
			{
				ofs << ", " << BuildAttributeList(field.attributes);
			}
			ofs << ")\n";
		}
	}
	for (const auto& method : info.methods)
	{
		std::string castExpr = "static_cast<" + method.returnType + "(" + info.name + "::*)(";
		for (size_t i = 0; i < method.parameters.size(); i++)
		{
			if (i > 0) castExpr += ", ";
			castExpr += method.parameters[i].type; // 引数の型をカンマ区切りで連結
		}
		if (method.parameters.empty())
		{
			castExpr += "void"; // 引数なしは void として扱う
		}
		castExpr += ")";
		if (method.isConst)
		{
			castExpr += " const";
		}
		castExpr += ">";

		ofs << "    REGISTER_METHOD(" 
			<< info.name << ", "
			<< method.name << ", "
			<< method.returnType << ", "
			<< castExpr << "(&" << info.name << "::" << method.name << ")";
		
		if (!method.parameters.empty())
		{
			ofs << ", ";
			for (size_t i = 0; i < method.parameters.size(); i++)
			{
				const auto& param = method.parameters[i];
				ofs << param.type << " " << param.name;
				if (i < method.parameters.size() - 1)
					ofs << ", ";
				// TODO: デフォルト引数もマクロに渡す必要があるかもしれない（現状は無視している）
			}
		}
		else
		{
			ofs << ", void"; // 引数なしは void として扱う
		}
		
		ofs << ")\n";
	}
	ofs << "END_REGISTER(" << info.name << ")\n";
	ofs.close();
}

void Generater::GenerateSource(const ClassInfo& info, const std::string& outPath, const std::string& includePath, const std::string& relativeSolutionPath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}
	// クラス名
	std::string className = info.name;
	
	//ofs << "#include \"" << relativeSolutionPath << "/pch.h" << "\"\n";
	ofs << "#include \"" << includePath << "\"\n";
	ofs << "#include \"" << relativeSolutionPath << "/Engine/Scenes/Scene.h" << "\"\n";
	ofs << "#include \"" << relativeSolutionPath << "/Engine/Scenes/SceneManager.h" << "\"\n";
	// API 関数の実装はここに書く（例: オブジェクトIDからコンポーネントを取得する関数など）

	ofs << "static " << className << "* Get" << className << "ById(uint64_t objectId)\n{\n";
	ofs << "    Scene* scene = SceneManager::GetLoadingSceneOrCurrentScene();\n";
	ofs << "    if (!scene) return nullptr;\n";
	ofs << "    const auto& compMap = scene->GetObjectManager()->GetComponentCacheMap();\n";
	ofs << "    if (compMap.contains(ObjectId::FromValue(objectId))) {\n";
	ofs << "        if (auto compPtr = compMap.at(ObjectId::FromValue(objectId)).lock()) {\n";
	ofs << "            if (auto ptr = dynamic_cast<" << className << "*>(compPtr.get())) {\n";
	ofs << "                return ptr;\n";
	ofs << "            }\n";
	ofs << "        }\n";
	ofs << "    }\n";
	ofs << "    return nullptr;\n";
	ofs << "}\n";

	// フィールドの Get/Set 関数も同様に実装していく
	for (const auto& field : info.fields)
	{
		// 先頭大文字のフィールド名
		std::string capName = field.name;
		std::string fieldName = field.name;
		if (!capName.empty()) capName[0] = static_cast<char>(std::toupper(capName[0]));
		// フィールドの型
		bool fieldIsEnum = IsEnumType(field.type, knownEnums, typeMap);
		std::string exportType = fieldIsEnum ? "int" : field.type;

		// Get 関数
		bool isVectorType = (field.type == "Vector2" || field.type == "Vector3");
		std::string returnType = exportType;
		std::string inputSignature = "uint64_t objectId";

		// Vector2/Vector3 の場合は outValue 引数を追加して void を返す
		if (isVectorType)
		{
			returnType = "void";
			inputSignature += ", " + exportType + "* outValue";
		}

		ofs << "extern \"""C\"" << " __declspec(dllexport) " << returnType << " " << className << "_Get" << capName << "(" << inputSignature << ")\n{\n";
		ofs << "    if (auto comp = Get" << className << "ById(objectId)) {\n";
		if (isVectorType)
		{
			ofs << "        *outValue = comp->" << fieldName << ";\n";
			ofs << "    }\n";
		}
		else
		{
			if (fieldIsEnum)
			{
				// 列挙型は int にキャストして返す
				ofs << "        return static_cast<int>(comp->" << fieldName << ");\n";
			}
			else
			{
				ofs << "        return comp->" << fieldName << ";\n";
			}
			ofs << "    }\n";
			if (fieldIsEnum)
			{
				// 列挙型は int から元の型にキャストして戻り値を返す
				ofs << "    return static_cast<int>(" << field.type << "{}); // Return Value When an Object Is Not Found\n";
			}
			else
			{
				std::string returnValue = returnType + "{};";
				ofs << "    return " << returnValue << "// Return Value When an Object Is Not Found\n";
			}
		}
		ofs << "}\n";
		// Set 関数
		ofs << "extern \"""C\"" << " __declspec(dllexport) void " << className << "_Set" << capName << "(uint64_t objectId, "
			<< exportType << " value)\n{\n";
		ofs << "    if (auto comp = Get" << className << "ById(objectId)) {\n";
		if (fieldIsEnum)
		{
			// 列挙型は int から元の型にキャストして代入
			ofs << "        comp->" << fieldName << " = static_cast<" << field.type << ">(value);\n";
		}
		else
		{
			ofs << "        comp->" << fieldName << " = value;\n";
		}
		ofs << "    }\n";
		ofs << "}\n";
	}

	// 他の API 関数も同様に実装していく
	for (const auto& method : info.methods)
	{
		if (method.isPropertyAccessor) continue; // プロパティのアクセサリはスキップ

		bool retIsEnum = IsEnumType(method.returnType, knownEnums, typeMap);
		std::string exportRetType = retIsEnum ? "int" : method.returnType;

		// Vector2/Vector3 の場合は outValue 引数を追加して void を返す
		bool isVectorType = (method.returnType == "Vector2" || method.returnType == "Vector3");
		std::string returnType = exportRetType;
		std::string inputSignature = "uint64_t objectId";

		// Vector2/Vector3 の場合は outValue 引数を追加して void を返す
		if (isVectorType)
		{
			returnType = "void";
			inputSignature += ", " + exportRetType + "* outValue";
		}

		ofs << "extern \"""C\"" << " __declspec(dllexport) " << returnType << " " << className << "_" << method.name << "(" << inputSignature;
		for (const auto& param : method.parameters)
		{
			bool paramIsEnum = IsEnumType(param.type, knownEnums, typeMap);
			std::string exportParamType = paramIsEnum ? "int" : param.type;

			ofs << ", " << exportParamType << " " << param.name;
		}
		ofs << ")\n{\n";
		ofs << "    if (auto comp = Get" << className << "ById(objectId)) {\n";
		ofs << "        // Implementing API processing using `comp`\n";
		// メソッドの呼び出し
		if (returnType != "void")
		{
			if (retIsEnum)
			{
				ofs << "        return static_cast<int>(comp->" << method.name << "(";
			}
			else
			{
				ofs << "        return comp->" << method.name << "(";
			}
		}
		else
		{
			ofs << "        comp->" << method.name << "(";
		}
		// 引数を渡す
		for (size_t i = 0; i < method.parameters.size(); i++)
		{
			const auto& param = method.parameters[i];
			if (IsEnumType(param.type, knownEnums, typeMap))
			{
				// int から元の列挙型にキャストして渡す
				ofs << "static_cast<" << param.type << ">(" << param.name << ")";
			}
			else
			{
				ofs << param.name;
			}
			if (i < method.parameters.size() - 1)
				ofs << ", ";
		}
		ofs << ");\n";
		ofs << "    }\n";
		if (returnType != "void")
		{
			if (retIsEnum)
			{
				ofs << "    return static_cast<int>(" << returnType << "{}); // Return Value When an Object Is Not Found\n";
			}
			else
			{
				std::string returnValue = returnType + "{};";
				ofs << "    return " << returnValue << "// Return Value When an Object Is Not Found\n";
			}
		}
		ofs << "}\n";
	}


	
	ofs.close();
}

void Generater::GenerateEnum(const EnumInfo& info, const std::string& outPath, const std::string& includePath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}

	ofs << "#pragma once\n\n";
	ofs << "#include \"" << includePath << "\"\n";

	// 列挙型の登録コードを生成
	ofs << "REGISTER_ENUM(" << info.name << ")\n";
	ofs << "    UNDERLYING_TYPE(" << info.underlyingType << ")\n";
	for (const auto& value : info.values)
	{
		ofs << "    ENUM_VALUE(\"" << value.name << "\"";
		if (value.hasExplicitValue)
		{
			ofs << ", " << "static_cast<" << info.underlyingType << ">(" << value.value << ")";
		}
		else // 値が明示されていない場合は自動カウンターを使用
		{
			ofs << ", " << "static_cast<" << info.underlyingType << ">(" << info.name << "::" << value.name << ")";
		}
		ofs << ")\n";
	}
	ofs << "END_REGISTER_ENUM(" << info.name << ")\n";
	ofs.close();
}

void Generater::GenerateStruct(const StructInfo& info, const std::string& outPath, const std::string& includePath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}

	ofs << "#pragma once\n\n";
	ofs << "#include \"" << includePath << "\"\n";

	// 構造体の登録コードを生成
	ofs << "REGISTER_STRUCT(" << info.name << ")\n";
	for (const auto& field : info.fields)
	{
		ofs << "    STRUCT_FIELD(" << info.name << ", " << field.name << ", " << field.type << ")\n";
	}
	ofs << "END_REGISTER_STRUCT(" << info.name << ")\n";
	ofs.close();
}
