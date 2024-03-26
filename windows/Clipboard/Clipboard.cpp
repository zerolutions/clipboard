#include "pch.h"
#include "Clipboard.h"
#include <Unicode.h>

namespace NativeClipboard {
  void ClipboardModule::GetString(React::ReactPromise<std::string>&& promise) noexcept {
    auto dataPackageView = datatransfer::Clipboard::GetContent();
    if (dataPackageView.Contains(datatransfer::StandardDataFormats::Text())) {
      dataPackageView.GetTextAsync().Completed([promise, dataPackageView](IAsyncOperation<winrt::hstring> info, AsyncStatus status) {
        if (status == AsyncStatus::Completed) {
          auto text = winrt::to_string(info.GetResults());
          promise.Resolve(text);
        }
        else {
          promise.Reject("Failure");
        }
      });
      return;
    }
    promise.Resolve("");
  }

  void ClipboardModule::SetString(std::string const& str) noexcept
  {
    _context.UIDispatcher().Post([str](){
      datatransfer::DataPackage dataPackage{};
      dataPackage.SetText(winrt::to_hstring(str));
      datatransfer::Clipboard::SetContent(dataPackage);
    });
  }

  void ClipboardModule::GetHtml(React::ReactPromise<std::string>&& promise) noexcept {
    auto dataPackageView = datatransfer::Clipboard::GetContent();
    if (dataPackageView.Contains(datatransfer::StandardDataFormats::Html())) {
      dataPackageView.GetHtmlFormatAsync().Completed([promise, dataPackageView](IAsyncOperation<winrt::hstring> info, AsyncStatus status) {
        if (status == AsyncStatus::Completed) {
          auto html = winrt::to_string(info.GetResults());
          promise.Resolve(html);
        }
        else {
          promise.Reject("Failure");
        }
      });
      return;
    }
    promise.Resolve("");
  }

  void ClipboardModule::SetHtmlAndString(std::string const& html, std::string const& str) noexcept {
    _context.UIDispatcher().Post([html, str](){
      std::string cfHtml = html;

      if (cfHtml.find("Version:") == 0) {
        for (int i = 0; i < 5; ++i) {
          size_t pos = cfHtml.find('\n');
          if (pos != std::string::npos) {
              cfHtml.erase(0, pos + 1);
          } else {
              break;
          }
        }
      }

      std::string startFragmentStr = "<!--StartFragment-->";
      std::string endFragmentStr = "<!--EndFragment-->";
      std::string bodyStartStr = "<body";
      std::string bodyEndStr = "</body>";
      std::string htmlStartStr = "<html";
      std::string htmlEndStr = "</html>";

      if (cfHtml.find(startFragmentStr) == std::string::npos || cfHtml.find(endFragmentStr) == std::string::npos) {
          cfHtml = startFragmentStr + cfHtml + endFragmentStr;
      }

      if (cfHtml.find(bodyStartStr) == std::string::npos || cfHtml.find(bodyEndStr) == std::string::npos) {
          cfHtml = bodyStartStr+'>' + cfHtml + bodyEndStr;
      }
      
      if (cfHtml.find(htmlStartStr) == std::string::npos || cfHtml.find(htmlEndStr) == std::string::npos) {
          cfHtml = htmlStartStr+'>' + cfHtml + htmlEndStr;
      }

      cfHtml = 
        "Version:1.0\r\n"
        "StartHTML:0000000000\r\n"
        "EndHTML:0000000000\r\n"
        "StartFragment:0000000000\r\n"
        "EndFragment:0000000000\r\n"
        + cfHtml;

      int startHtml = cfHtml.find(htmlStartStr);
      int endHtml = cfHtml.find(htmlEndStr) + 7;
      int startFragment = cfHtml.find(startFragmentStr) + startFragmentStr.length();
      int endFragment = cfHtml.find(endFragmentStr);

      cfHtml.replace(cfHtml.find("StartHTML:0000000000"), 20, "StartHTML:" + std::to_string(startHtml).insert(0, 10 - std::to_string(startHtml).length(), '0'));
      cfHtml.replace(cfHtml.find("EndHTML:0000000000"), 18, "EndHTML:" + std::to_string(endHtml).insert(0, 10 - std::to_string(endHtml).length(), '0'));
      cfHtml.replace(cfHtml.find("StartFragment:0000000000"), 24, "StartFragment:" + std::to_string(startFragment).insert(0, 10 - std::to_string(startFragment).length(), '0'));
      cfHtml.replace(cfHtml.find("EndFragment:0000000000"), 22, "EndFragment:" + std::to_string(endFragment).insert(0, 10 - std::to_string(endFragment).length(), '0'));

      auto dataPackage = datatransfer::DataPackage();
      dataPackage.SetHtmlFormat(winrt::to_hstring(cfHtml));
      if (!str.empty()) {
          dataPackage.SetText(winrt::to_hstring(str));
      }
      datatransfer::Clipboard::SetContent(dataPackage);
    });
  }

  void ClipboardModule::AddListener(std::string const& event) noexcept
  {
    _listenerCount++;
  }

  void ClipboardModule::RemoveListeners(int count) noexcept
  {
    _listenerCount--;
    if (_listenerCount == 0) {
      // Disconnect any native eventing here
    }
  }
}
