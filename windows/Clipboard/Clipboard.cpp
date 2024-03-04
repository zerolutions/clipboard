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
      std::string cfHtml = "";

      if (html.find("Version:") == 0) {
        cfHtml = html;
      } else {
        std::string startFragmentStr = "<!--StartFragment-->";
        std::string endFragmentStr = "<!--EndFragment-->";
        std::string htmlContent = "<html>" + startFragmentStr + "<body>" + html + "</body>" + endFragmentStr + "</html>";

        cfHtml = 
          "Version:1.0\r\n"
          "StartHTML:0000000000\r\n"
          "EndHTML:0000000000\r\n"
          "StartFragment:0000000000\r\n"
          "EndFragment:0000000000\r\n"
          + htmlContent;

        int startHtml = cfHtml.find("<html>");
        int endHtml = cfHtml.find("</html>") + 7;
        int startFragment = cfHtml.find(startFragmentStr) + startFragmentStr.length();
        int endFragment = cfHtml.find(endFragmentStr);

        cfHtml.replace(cfHtml.find("StartHTML:0000000000"), 20, "StartHTML:" + std::to_string(startHtml).insert(0, 10 - std::to_string(startHtml).length(), '0'));
        cfHtml.replace(cfHtml.find("EndHTML:0000000000"), 18, "EndHTML:" + std::to_string(endHtml).insert(0, 10 - std::to_string(endHtml).length(), '0'));
        cfHtml.replace(cfHtml.find("StartFragment:0000000000"), 24, "StartFragment:" + std::to_string(startFragment).insert(0, 10 - std::to_string(startFragment).length(), '0'));
        cfHtml.replace(cfHtml.find("EndFragment:0000000000"), 22, "EndFragment:" + std::to_string(endFragment).insert(0, 10 - std::to_string(endFragment).length(), '0'));
      }
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
