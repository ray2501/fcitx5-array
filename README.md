# fcitx5-array

Array 30 input method engine for Fcitx 5 project

[fcitx5-table-extra](https://github.com/fcitx/fcitx5-table-extra)
已有包含行列輸入法的表格可以使用，但沒有提供一級簡碼、二級簡碼以及符號輸入
等功能。

[RIME](https://rime.im/) 支援多個平台，包含 Fcitx 在內，
[rime-array](https://github.com/rime/rime-array) 則是 RIME 的方案，
並且提供了豐富的功能，不過缺少一級簡碼和二級簡碼功能。

本專案嘗試在 Fctix 5 上實作[行列輸入法](https://www.facebook.com/array.com.tw/)
並自 [ibus-array](https://github.com/lexical/ibus-array) 移植下列功能：
- 一級簡碼與二級簡碼
- 符號輸入（「W+數字鍵」符號選單）
- 查詢鍵「?」，代表任意一個行列 30 鍵

並且嘗試支援下列的功能：
- 查詢鍵「*」，代表零個或者更多個行列 30 鍵（目前本專案的實作限制為不能
作為第一個按鍵）
- 詞彙輸入（詞庫來自
[array30 行列輸入法（30 鍵版）表格](https://github.com/gontera/array30)，
詞彙以 ' 作尾碼）
- 支援自訂按鍵可以暫時使用 Fcitx 5 QuickPhrase 輸入（建議使用 Super + `）
- 支援 Fcitx 5 chttrans 模組（繁簡轉換），預設設為 disable
- 嘗試當應用程式支援 surrounding text，支援使用者使用 Ctrl+Alt+E 呼叫反查
行列碼功能（按鍵可自訂，目前限制只能查詢已選字中的第一個字）
- 使用 Fcitx 5 notifications 模組實作特別碼通知，預設為 disable

與 ibus-array 相同，輸入法表格使用 SQLite3。授權方式也一樣為 GPL2+。

## 安裝

Ubuntu Mate 22.04 LTS 的環境，在設好 Language Support 以後，
確定有安裝 Fcitx5，使用 `im-config -n fcitx5` 設定為預設輸入法。

openSUSE Leap 15.4 目前 Fcitx 預設安裝的版本為 4，如果要使用 Fcitx 5，需要使用下列的指令安裝
（指令包含安裝輸入法）：
```
sudo zypper addrepo https://download.opensuse.org/repositories/M17N/15.4/M17N.repo
sudo zypper refresh
sudo zypper install fcitx5 fcitx5-table-extra fcitx5-zhuyin fcitx5-chewing
```

fcitx5-array 採用 CMake，
要自行編譯的話在 Ubuntu Mate 22.04 LTS 需要安裝下列的開發用套件：
```
sudo apt install cmake extra-cmake-modules gettext \
    libfcitx5core-dev  fcitx5-modules-dev libfcitx5config-dev libfcitx5utils-dev \
    libsqlite3-dev libfmt-dev
```
如果是 openSUSE 需要安裝下列的開發用套件：
```
sudo zypper in cmake extra-cmake-modules gettext \
    fcitx5-devel fcitx5-chinese-addons-devel sqlite3-devel fmt-devel
```

然後在專案的目錄下執行以下指令：
```
mkdir -p build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```
在 Ubuntu Mate 22.04 LTS 安裝後，需要執行以下指令，fcitx5-array icon 才會正確出現在 fcitx5 選單中。
```
sudo update-icon-caches /usr/share/icons/*
```
如果要移除，在 build 目錄下執行以下指令：
```
sudo make uninstall
```

## 工具

原始碼 data 目錄下除了輸入法表格檔案 array.db，還有來自
[array30 行列輸入法（30 鍵版）表格](https://github.com/gontera/array30)的原始檔案，
以及將原始檔案的資料匯入輸入法表格檔案 array.db 的工具程式
（修改自 ibus-array，只是因應我個人的需要而有少許的修改），
如果使用者因為個人的需求而想自行更新，可以使用工具程式更新 array.db。

工具程式需要環境有安裝 Python 3 才能夠工作。
cin2sqlite.py 負責更新 main，也就是主要的輸入法表格，
使用範例如下：
```
./cin2sqlite.py array30-OpenVanilla-big-v2023-1.0-20230211.cin
```
updateSimple.py 負責更新 simple，也就是一級簡碼與二級簡碼的表格，使用範例如下：
```
./updateSimple.py array30_simplecode.cin
```
updatePhrase.py 負責更新 phrase，也就是詞庫的表格，使用範例如下：
```
./updatePhrase.py array30-phrase-20210725.txt
```

