# リリースパッケージ作成手順

本書は、mm2hackのバージョン表記を確認し、配布用ZIPとSHA-256ファイルを
作成するまでの手順を定める。

リリース全体の確認項目については、
`【ゲーム開発】romhacking_PG-003_リリース前チェックリスト.md`も参照すること。

## 1. 作業場所

PowerShellを開き、Gitリポジトリのルートへ移動する。

```powershell
Set-Location "D:\00_MainProjects\probable-enigma"
```

以下のコマンドは、原則としてこのディレクトリから実行する。

## 2. バージョン番号の確認

### 2.1 アプリケーション表示用バージョン

`RetroEngine/mm2hack/mm2hack.env`の`MM2HACK_VERSION`を確認する。

```text
$MM2HACK_VERSION v0.0.3
```

正式リリースでは、`-beta`などのプレリリース識別子が残っていないことを
確認する。

### 2.2 Windowsリソースのバージョン

`RetroEngine/mm2hack/mm2hack.rc`の以下の4項目を確認する。

```rc
FILEVERSION 0,0,3,0
PRODUCTVERSION 0,0,3,0
VALUE "FileVersion", "0.0.3.0"
VALUE "ProductVersion", "0.0.3.0"
```

一括確認には、次のコマンドを使用できる。

```powershell
rg -n 'MM2HACK_VERSION|FILEVERSION|PRODUCTVERSION|FileVersion|ProductVersion' `
    .\RetroEngine\mm2hack\mm2hack.env `
    .\RetroEngine\mm2hack\mm2hack.rc
```

`SystemConfig::kCurrentSaveVersion`はセーブファイル形式のバージョンであり、
アプリケーションのリリース番号ではない。セーブ形式を変更していない場合は
更新しないこと。

## 3. リリース文書の更新

### 3.1 CHANGELOG.md

リポジトリルートの`CHANGELOG.md`へ、新しいバージョンを追記する。
新しいバージョンは既存の最新版より後ろへ追加し、次の項目を記載する。

```markdown
## [0.0.3] - YYYY/MM/DD

### Added
- Added feature summary.

### Changed
- Changed behavior summary.

### Fixed
- Fixed issue summary.

### Known Issues
- Remaining known issue.
```

該当する変更がない見出しは省略してよい。

### 3.2 RELEASE_NOTES.md

リポジトリルートの`RELEASE_NOTES.md`では、`Common Information`の後、
旧バージョンより前へ新しいバージョンを追加する。

```markdown
## v0.0.3 - YYYY/MM/DD

### Highlights
- Major user-facing highlight.

### Downloads
- Windows (ZIP): download URL

### What's Changed (Summary)
- **Features**:
  - User-facing feature.
- **Improvements**:
  - User-facing improvement.

### Known Issues
- Remaining known issue.

### Checksums
- SHA256_VALUE  *probable-enigma_mm2hack_demo_v003.zip
```

ダウンロードURLとSHA-256は、配布ZIPの完成後に確定する。

## 4. 配布フォルダーの構成

リポジトリルートに`dist`を作成し、その中へバージョンごとの
パッケージフォルダーを用意する。

```text
dist/
├─ probable-enigma_mm2hack_demo_v003/
│  ├─ mm2hack.exe
│  ├─ assets/
│  ├─ mm2hack.env
│  ├─ readme.txt
│  └─ licenses/
│     ├─ UNLICENSE.txt
│     ├─ THIRD_PARTY_NOTICES.md
│     ├─ nlohmann-json-MIT.txt
│     └─ Apache-2.0.txt
├─ probable-enigma_mm2hack_demo_v003.zip
└─ probable-enigma_mm2hack_demo_v003.zip.sha256
```

`dist`自体は圧縮しない。圧縮対象は、次のフォルダーだけである。

```text
dist/probable-enigma_mm2hack_demo_v003/
```

配布物には、`.pdb`、`.obj`、`.sav`、ログ、スクリーンショット、
Visual Studioの中間生成物、開発専用ファイルを含めないこと。

## 5. PowerShell変数の設定

リリースごとに、最初の`$releaseVersion`だけを変更する。

```powershell
$releaseVersion = "0.0.3"
$compactVersion = $releaseVersion.Replace(".", "")
$packageName = "probable-enigma_mm2hack_demo_v$compactVersion"
$distPath = Join-Path (Get-Location) "dist"
$packagePath = Join-Path $distPath $packageName
$archivePath = Join-Path $distPath "$packageName.zip"
$checksumPath = "$archivePath.sha256"
```

新しいパッケージフォルダーを作成する。既存の同名フォルダーまたはZIPが
ある場合は処理を停止し、内容を確認してから対応する。

```powershell
if ((Test-Path -LiteralPath $packagePath) -or
    (Test-Path -LiteralPath $archivePath) -or
    (Test-Path -LiteralPath $checksumPath))
{
    throw "Release output already exists: $packageName"
}

New-Item -ItemType Directory -Path $packagePath -Force | Out-Null
```

作成した`$packagePath`へ、Releaseビルドした実行ファイル、実行時リソース、
環境設定、readme、ライセンス文書を配置する。

## 6. ZIPファイルの生成

パッケージ内容を最終確認した後、次のコマンドでZIPを生成する。

```powershell
Compress-Archive `
    -LiteralPath $packagePath `
    -DestinationPath $archivePath `
    -CompressionLevel Optimal
```

生成されたZIPには`dist`を含めず、`$packageName`フォルダーだけが
格納されていることを確認する。

## 7. SHA-256ファイルの生成

ZIP完成後、次のコマンドでSHA-256ファイルを生成する。

```powershell
$archiveFileName = Split-Path -Leaf $archivePath
$archiveHash = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash

"$archiveHash  *$archiveFileName" |
    Set-Content -LiteralPath $checksumPath -Encoding ascii
```

生成例：

```text
0123456789ABCDEF...  *probable-enigma_mm2hack_demo_v003.zip
```

各ファイル単位の`CHECKSUMS.txt`は、ZIPを一括配布する現在の方式では
必須ではない。

## 8. SHA-256の照合

アップロード前に、SHA-256ファイルの値とZIPから再計算した値を照合する。

```powershell
$expectedHash = (
    (Get-Content -LiteralPath $checksumPath -Raw).Trim() -split '\s+'
)[0]
$actualHash = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash

if (![string]::Equals(
        $expectedHash,
        $actualHash,
        [System.StringComparison]::OrdinalIgnoreCase))
{
    throw "SHA-256 verification failed."
}

Write-Host "SHA-256 verification passed: $actualHash"
```

ZIPを作り直した場合は、SHA-256ファイルと`RELEASE_NOTES.md`の
チェックサムも必ず更新する。

## 9. アップロード前の最終確認

- [ ] Release構成でビルドしたか
- [ ] 通常起動と主要操作を確認したか
- [ ] 自動テストが成功したか
- [ ] ZIP内に不要な開発ファイルがないか
- [ ] ライセンス文書を同梱したか
- [ ] ZIPとSHA-256の照合に成功したか
- [ ] `RELEASE_NOTES.md`のURLとSHA-256が完成物と一致しているか
- [ ] ZIPと`.zip.sha256`の両方をアップロード対象にしたか
