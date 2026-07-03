#include "pch.h"
#include "VSProjectReloader.h"
#include <windows.h>
#include <atlbase.h> // CComPtr, CComBSTR
#include <filesystem>
#include "Engine/Editor/Console.h"

// --- IDispatch ヘルパー ---

// プロパティ取得 (get_Xxx)
static HRESULT DispGetProperty(IDispatch* pDisp, LPCOLESTR name, VARIANT* pResult)
{
    DISPID dispId;
    BSTR bstrName = SysAllocString(name);
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &bstrName, 1,
        LOCALE_USER_DEFAULT, &dispId);
    SysFreeString(bstrName);
    if (FAILED(hr)) return hr;

    DISPPARAMS dp = {};
    return pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_PROPERTYGET, &dp, pResult, nullptr, nullptr);
}

// メソッド呼び出し (引数あり)
static HRESULT DispCall(IDispatch* pDisp, LPCOLESTR name,
    VARIANT* args, int argCount, VARIANT* pResult = nullptr)
{
    DISPID dispId;
    BSTR bstrName = SysAllocString(name);
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &bstrName, 1,
        LOCALE_USER_DEFAULT, &dispId);
    SysFreeString(bstrName);
    if (FAILED(hr)) return hr;

    // IDispatch::Invoke は引数を逆順で渡す
    DISPPARAMS dp = {};
    dp.cArgs = argCount;
    dp.rgvarg = args; // 呼び出し元で逆順にして渡すこと

    return pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD, &dp, pResult, nullptr, nullptr);
}


bool VSProjectReloader::ReloadProject(const std::wstring& vcxprojPath)
{
	// STA スレッドで COM を初期化
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	bool result = false;

    do {
		// ROT から Visual Studio の DTE オブジェクトを取得
		CComPtr<IRunningObjectTable> rot;
		if (FAILED(GetRunningObjectTable(0, &rot))) break;

		CComPtr<IEnumMoniker> enumMoniker;
		if (FAILED(rot->EnumRunning(&enumMoniker))) break;

        CComPtr<IDispatch> dte;
		CComPtr<IMoniker> moniker;
        while (enumMoniker->Next(1, &moniker, nullptr) == S_OK) {
            CComPtr<IBindCtx> bindCtx;
            if (FAILED(CreateBindCtx(0, &bindCtx))) continue;

			// モニカーから表示名を取得
            LPOLESTR displayName;
            if (FAILED(moniker->GetDisplayName(bindCtx, nullptr, &displayName))) continue;
            // DTE オブジェクトは "VisualStudio.DTE" で始まる ROT エントリとして登録されている
            std::wstring name(displayName ? displayName : L"");
            CoTaskMemFree(displayName);


            if (name.find(L"VisualStudio.DTE") != std::wstring::npos) {
                // 見つかった DTE オブジェクトを ROT から取得
                CComPtr<IUnknown> unknown;
                if (SUCCEEDED(rot->GetObject(moniker, &unknown))) {
                    unknown->QueryInterface(IID_IDispatch, (void**)&dte);
                }
			}
			moniker.Release();
			if (dte) break; // 最初に見つかった DTE を使う
		}
		if (!dte) break;

		//// DTE の Solution オブジェクトを取得
		//CComVariant varSolution;
		//if (FAILED(DispGetProperty(dte, L"Solution", &varSolution))) break;
		//if (varSolution.vt != VT_DISPATCH || !varSolution.pdispVal) break;
		//CComPtr<IDispatch> solution = varSolution.pdispVal;

		//// Solution オブジェクトの Projects コレクションを取得
		//CComVariant varProjects;
		//if (FAILED(DispGetProperty(solution, L"Projects", &varProjects))) break;
		//if (varProjects.vt != VT_DISPATCH || !varProjects.pdispVal) break;
		//CComPtr<IDispatch> projects = varProjects.pdispVal;

		//// Projects コレクションを列挙して、vcxprojPath と同じプロジェクトを探す
		//CComVariant varCount;
		//if (FAILED(DispGetProperty(projects, L"Count", &varCount))) break;
		//long count = varCount.lVal;

  //      // 対象のプロジェクトを検索
		//CComPtr<IDispatch> targetProject;
  //      std::wstring targetFullName;

  //      for (long i = 1; i <= count; ++i)
  //      {
		//	VARIANT argIndex;
		//	VariantInit(&argIndex);
		//	argIndex.vt = VT_I4;
		//	argIndex.lVal = i;

		//	// Projects コレクションの Item(i) でプロジェクトを取得
		//	CComVariant varProject;
		//	if (FAILED(DispCall(projects, L"Item", &argIndex, 1, &varProject))) continue;
		//	if (varProject.vt != VT_DISPATCH || !varProject.pdispVal) continue;

		//	CComPtr<IDispatch> project = varProject.pdispVal;
		//	CComVariant varFullName;
		//	if (FAILED(DispGetProperty(project, L"FullName", &varFullName))) continue;
		//	if (varFullName.vt != VT_BSTR || !varFullName.bstrVal) continue;

		//	std::wstring projectPath(varFullName.bstrVal, SysStringLen(varFullName.bstrVal));
		//	std::error_code ec;
		//	if (std::filesystem::equivalent(projectPath, vcxprojPath, ec)) {
		//		targetProject = project;
		//		targetFullName = projectPath;
		//		break;
		//	}
  //      }
		//if (!targetProject) {
		//	Console::LogError("Project not found in Visual Studio: " + std::string(vcxprojPath.begin(), vcxprojPath.end()));
		//	break;
		//}

		//// プロジェクトをアンロード
		//VARIANT argUnload;
		//VariantInit(&argUnload);
		//argUnload.vt = VT_DISPATCH;
		//argUnload.pdispVal = targetProject;
		//argUnload.pdispVal->AddRef();

		//HRESULT hrUnload = DispCall(solution, L"UnloadProject", &argUnload, 1);
		//if (FAILED(hrUnload)) {
		//	wchar_t buf[64];
		//	swprintf_s(buf, L"0x%08X", (unsigned)hrUnload);
		//	Console::LogError("Failed to unload project (HRESULT: "
		//		+ std::string(buf, buf + wcslen(buf)) + "): "
		//		+ std::string(targetFullName.begin(), targetFullName.end()));
		//	break;
		//}

		//// プロジェクトをリロード
		//VARIANT argReload;
		//VariantInit(&argReload);
		//argReload.vt = VT_BSTR;
		//argReload.bstrVal = SysAllocString(targetFullName.c_str());

		//if (FAILED(DispCall(solution, L"ReloadProject", &argReload, 1))) {
		//	Console::LogError("Failed to reload project: " + std::string(targetFullName.begin(), targetFullName.end()));
		//	break;
		//}
		//SysFreeString(argReload.bstrVal);

        // --- 2. UIHierarchy でソリューションエクスプローラーのプロジェクトノードを選択 ---
        // dte.Windows.Item("{3AE79031-E1BC-11D0-8F78-00A0C9110057}") で SolutionExplorer を取得
        CComVariant varWindows;
        if (FAILED(DispGetProperty(dte, L"Windows", &varWindows))) break;
        if (varWindows.vt != VT_DISPATCH) break;
        CComPtr<IDispatch> windows = varWindows.pdispVal;

        // SolutionExplorer の GUID
        VARIANT argWinItem;
        VariantInit(&argWinItem);
        argWinItem.vt = VT_BSTR;
        argWinItem.bstrVal = SysAllocString(L"{3AE79031-E1BC-11D0-8F78-00A0C9110057}");
        CComVariant varSEWindow;
        HRESULT hrWin = DispCall(windows, L"Item", &argWinItem, 1, &varSEWindow);
        SysFreeString(argWinItem.bstrVal);
        if (FAILED(hrWin) || varSEWindow.vt != VT_DISPATCH) break;
        CComPtr<IDispatch> seWindow = varSEWindow.pdispVal;

        // Window.Object → UIHierarchy
        CComVariant varUIHier;
        if (FAILED(DispGetProperty(seWindow, L"Object", &varUIHier))) break;
        if (varUIHier.vt != VT_DISPATCH) break;
        CComPtr<IDispatch> uiHier = varUIHier.pdispVal;

        // UIHierarchy.UIHierarchyItems → ルートアイテム群
        CComVariant varRootItems;
        if (FAILED(DispGetProperty(uiHier, L"UIHierarchyItems", &varRootItems))) break;
        if (varRootItems.vt != VT_DISPATCH) break;
        CComPtr<IDispatch> rootItems = varRootItems.pdispVal;

        // Item(1) = ソリューションノード
        VARIANT argOne;
        VariantInit(&argOne);
        argOne.vt = VT_I4; argOne.lVal = 1;
        CComVariant varSlnNode;
        if (FAILED(DispCall(rootItems, L"Item", &argOne, 1, &varSlnNode))) break;
        if (varSlnNode.vt != VT_DISPATCH) break;
        CComPtr<IDispatch> slnNode = varSlnNode.pdispVal;

        // ソリューションノードの子 = プロジェクトノード群
        CComVariant varProjItems;
        if (FAILED(DispGetProperty(slnNode, L"UIHierarchyItems", &varProjItems))) break;
        if (varProjItems.vt != VT_DISPATCH) break;
        CComPtr<IDispatch> projItems = varProjItems.pdispVal;

        CComVariant varProjCount;
        if (FAILED(DispGetProperty(projItems, L"Count", &varProjCount))) break;

        // プロジェクト名でノードを探す
        std::filesystem::path targetPath(vcxprojPath);
        std::wstring targetStem = targetPath.stem().wstring(); // "CurryEngine"

        CComPtr<IDispatch> targetNode;
        for (long i = 1; i <= varProjCount.lVal; ++i) {
            VARIANT argIdx;
            VariantInit(&argIdx);
            argIdx.vt = VT_I4; argIdx.lVal = i;
            CComVariant varNode;
            if (FAILED(DispCall(projItems, L"Item", &argIdx, 1, &varNode))) continue;
            if (varNode.vt != VT_DISPATCH) continue;

            CComPtr<IDispatch> node = varNode.pdispVal;
            CComVariant varNodeName;
            if (FAILED(DispGetProperty(node, L"Name", &varNodeName))) continue;
            if (varNodeName.vt != VT_BSTR) continue;

            std::wstring nodeName(varNodeName.bstrVal);
            if (nodeName == targetStem) {
                targetNode = node;
                break;
            }
        }
        if (!targetNode) {
            Console::LogError("Project node not found in SolutionExplorer");
            break;
        }

        // --- 3. ノードを選択状態にする ---
        VARIANT argSelect;
        VariantInit(&argSelect);
        argSelect.vt = VT_BOOL;
        argSelect.boolVal = VARIANT_TRUE;
        DispCall(targetNode, L"Select", &argSelect, 1);

        // --- 4. ExecuteCommand でアンロード → リロード ---
        auto ExecuteCommand = [&](const wchar_t* cmd) -> HRESULT {
            VARIANT args[2];
            VariantInit(&args[0]);
            args[0].vt = VT_BSTR;
            args[0].bstrVal = SysAllocString(L""); // CommandArgs (逆順で第2引数)
            VariantInit(&args[1]);
            args[1].vt = VT_BSTR;
            args[1].bstrVal = SysAllocString(cmd); // Command (逆順で第1引数)
            HRESULT hr = DispCall(dte, L"ExecuteCommand", args, 2);
            SysFreeString(args[0].bstrVal);
            SysFreeString(args[1].bstrVal);
			// コマンドの実行に失敗した場合はエラーをログに出す
            if (FAILED(hr)) {
                wchar_t buf[64];
                swprintf_s(buf, L"0x%08X", (unsigned)hr);
                Console::LogError("Failed to execute command '" + std::string(cmd, cmd + wcslen(cmd)) + "' (HRESULT: "
                    + std::string(buf, buf + wcslen(buf)) + ")");
            }

            return hr;
            };

        if (FAILED(ExecuteCommand(L"Project.UnloadProject"))) {
            Console::LogError("Failed to execute UnloadProject");
            break;
        }

        // リロード前に少し待つ (UnloadProject が非同期の場合があるため)
        Sleep(200);

        if (FAILED(ExecuteCommand(L"Project.ReloadProject"))) {
            Console::LogError("Failed to execute ReloadProject");
            break;
        }

		result = true;
	} while (false);

	CoUninitialize();
	return result;
}