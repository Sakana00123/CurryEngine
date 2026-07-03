#pragma once
#include <Windows.h>
#include <ole2.h>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// ドロップターゲットに必要なコールバック関数をまとめた構造体
struct DropTargetCallbacks
{
	std::function<bool(const POINTL&)> isOverGrid; // ドロップ中のカーソルがアセットグリッド上にあるか
	std::function<void(const std::vector<fs::path>&)> onDrop; // ドロップされたファイルの処理
	std::function<void(bool)> setHovering; // ドロップターゲットがホバー状態かどうかを外部に通知するコールバック（引数はホバー状態）
	std::function<void()> onDragEnter; // ドロップがグリッドに入ったときのコールバック
	std::function<void()> onDragLeave; // ドロップがグリッド外に離れたときのコールバック
};

// ドロップターゲットクラス
class AssetBrowserDropTarget : public IDropTarget
{
public:
	explicit AssetBrowserDropTarget(const DropTargetCallbacks& callbacks)
		: refCount_(0), callbacks_(callbacks)	 {}

	// IUnknownインターフェースの実装
	ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(InterlockedIncrement(&refCount_)); }
	ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG count = static_cast<ULONG>(InterlockedDecrement(&refCount_));
		if (count == 0) delete this;
		return count;
	}
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (riid == IID_IUnknown || riid == IID_IDropTarget)
		{
			*ppvObject = static_cast<IDropTarget*>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	// IDropTargetインターフェースの実装
	HRESULT STDMETHODCALLTYPE DragEnter(
		IDataObject* pDataObj, DWORD grfKeyState,
		POINTL pt, DWORD* pdwEffect) override
	{
		callbacks_.onDragEnter();
		*pdwEffect = hasFiles(pDataObj) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		updateHover(pt);
		return S_OK;
	}
    HRESULT STDMETHODCALLTYPE DragOver(
        DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
    {
        updateHover(pt);
        *pdwEffect = callbacks_.isOverGrid(pt) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeave() override
    {
		callbacks_.onDragLeave();
        callbacks_.setHovering(false);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Drop(
        IDataObject* pDataObj, DWORD grfKeyState,
        POINTL pt, DWORD* pdwEffect) override
    {
        callbacks_.setHovering(false);

        if (!callbacks_.isOverGrid(pt))
        {
            *pdwEffect = DROPEFFECT_NONE;
            return S_OK;
        }

        // IDataObject からファイルパスを取り出す
        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stg = {};
        if (SUCCEEDED(pDataObj->GetData(&fmt, &stg)))
        {
            HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
            if (hDrop)
            {
                UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
                std::vector<fs::path> paths;
                paths.reserve(count);
                for (UINT i = 0; i < count; ++i)
                {
                    WCHAR buf[MAX_PATH];
                    if (DragQueryFileW(hDrop, i, buf, MAX_PATH))
                        paths.emplace_back(buf);
                }
                GlobalUnlock(stg.hGlobal);
                callbacks_.onDrop(std::move(paths));
            }
            ReleaseStgMedium(&stg);
        }

        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

private:
    void updateHover(POINTL pt) const
    {
        // スクリーン座標は AssetBrowser 側の assetGridScreenRect と比較
        callbacks_.setHovering(callbacks_.isOverGrid(pt));
    }

	bool hasFiles(IDataObject* pDataObj) const
    {
        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        return pDataObj->QueryGetData(&fmt) == S_OK;
    }

    volatile LONG refCount_;
    DropTargetCallbacks callbacks_;
};