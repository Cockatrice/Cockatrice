<?xml version="1.0" ?><!DOCTYPE TS><TS language="ko" version="2.1">
<context>
    <name>IntroPage</name>
    <message>
        <location filename="../src/oraclewizard.cpp" line="166"/>
        <source>Introduction</source>
        <translation>개요</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="167"/>
        <source>This wizard will import the list of sets, cards, and tokens that will be used by Cockatrice.
You will need to specify a URL or a filename that will be used as a source.</source>
        <translation>오라클은 코카트리스에서 사용할 확장판의 목록과 카드들의 정보를 갱신하는 프로그램입니다.
판본 목록 파일이 있는 웹 주소나 파일을 입력하신 후에 읽어온 목록에서 원하는 확장판을 선택해 불러올 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="171"/>
        <source>Language:</source>
        <translation>언어</translation>
    </message>
</context>
<context>
    <name>LoadSetsPage</name>
    <message>
        <location filename="../src/oraclewizard.cpp" line="219"/>
        <source>Source selection</source>
        <translation>확장판 목록 파일 주소 입력</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="220"/>
        <source>Please specify a source for the list of sets and cards. You can specify a URL address that will be downloaded or use an existing file from your computer.</source>
        <translation>확장판 목록 및 카드 정보가 들어있는 파일의 위치를 입력해 주세요. 
다운로드 할 수 있는 웹 주소나 컴퓨터에 저장되어 있는 파일을 선택 할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="224"/>
        <source>Download URL:</source>
        <translation>웹 주소:</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="225"/>
        <source>Local file:</source>
        <translation>파일 위치:</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="226"/>
        <source>Restore default URL</source>
        <translation>기본 주소로 복원</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="227"/>
        <source>Choose file...</source>
        <translation>파일 선택...</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="237"/>
        <source>Load sets file</source>
        <translation>확장판 목록 파일 불러오기</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="241"/>
        <source>Sets JSON file (*.json *.zip)</source>
        <translation>확장판 목록 JSON 파일 (*.json *.zip)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="243"/>
        <source>Sets JSON file (*.json)</source>
        <translation>확장판 목록 JSON 파일 (*.json)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="267"/>
        <location filename="../src/oraclewizard.cpp" line="287"/>
        <location filename="../src/oraclewizard.cpp" line="292"/>
        <location filename="../src/oraclewizard.cpp" line="331"/>
        <location filename="../src/oraclewizard.cpp" line="430"/>
        <location filename="../src/oraclewizard.cpp" line="451"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="267"/>
        <source>The provided URL is not valid.</source>
        <translation>잘못된 주소를 입력하셨습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="271"/>
        <source>Downloading (0MB)</source>
        <translation>다운로드 중 (0MB)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="287"/>
        <source>Please choose a file.</source>
        <translation>확장판 목록 파일을 선택해 주세요.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="292"/>
        <source>Cannot open file &apos;%1&apos;.</source>
        <translation>파일 &apos;%1&apos;을(를) 열 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="322"/>
        <source>Downloading (%1MB)</source>
        <translation>다운로드 중 (%1MB)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="331"/>
        <source>Network error: %1.</source>
        <translation>네트워크 오류 : %1.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="368"/>
        <source>Parsing file</source>
        <translation>목록 파싱중</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="385"/>
        <source>Failed to open Zip archive: %1.</source>
        <translation>압축파일 열기 실패 : %1.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="391"/>
        <source>Zip extraction failed: the Zip archive doesn&apos;t contain exactly one file.</source>
        <translation>압축 풀기 실패 : 압축 파일에 확장판 목록 파일 이외의 파일이 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="399"/>
        <source>Zip extraction failed: %1.</source>
        <translation>압축 풀기 실패 : %1.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="408"/>
        <source>Sorry, this version of Oracle does not support zipped files.</source>
        <translation>죄송합니다. 본 버전에서는 압축 파일을 지원하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="430"/>
        <source>Do you want to try to download a fresh copy of the uncompressed file instead?</source>
        <translation>압축되지 않은 확장판 목록을 대신 내려받으시겠습니까?</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="451"/>
        <source>The file was retrieved successfully, but it does not contain any sets data.</source>
        <translation>파일을 성공적으로 다운로드 하였으나 확장판 정보가 들어있지 않습니다.</translation>
    </message>
</context>
<context>
    <name>LoadTokensPage</name>
    <message>
        <location filename="../src/oraclewizard.cpp" line="578"/>
        <source>Tokens source selection</source>
        <translation>토큰 파일 주소 입력</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="579"/>
        <source>Please specify a source for the list of tokens. You can specify a URL address that will be downloaded or use an existing file from your computer.</source>
        <translation>토큰 목록 및 정보가 들어있는 파일의 위치를 입력해 주세요. 
다운로드 할 수 있는 웹 주소나 컴퓨터에 저장되어 있는 파일을 선택 할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="583"/>
        <source>Download URL:</source>
        <translation>웹 주소:</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="584"/>
        <source>Restore default URL</source>
        <translation>기본 주소로 복원</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="601"/>
        <location filename="../src/oraclewizard.cpp" line="646"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="601"/>
        <source>The provided URL is not valid.</source>
        <translation>잘못된 주소를 입력하셨습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="605"/>
        <source>Downloading (0MB)</source>
        <translation>다운로드 중 (0MB)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="637"/>
        <source>Downloading (%1MB)</source>
        <translation>다운로드 중 (%1MB)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="646"/>
        <source>Network error: %1.</source>
        <translation>네트워크 오류 : %1.</translation>
    </message>
</context>
<context>
    <name>OracleImporter</name>
    <message>
        <location filename="../src/oracleimporter.cpp" line="318"/>
        <source>Dummy set containing tokens</source>
        <translation>토큰 정보가 들어있는 더미 확장판</translation>
    </message>
</context>
<context>
    <name>OracleWizard</name>
    <message>
        <location filename="../src/oraclewizard.cpp" line="73"/>
        <source>Oracle Importer</source>
        <translation>오라클</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="74"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
</context>
<context>
    <name>SaveSetsPage</name>
    <message>
        <location filename="../src/oraclewizard.cpp" line="483"/>
        <location filename="../src/oraclewizard.cpp" line="537"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="483"/>
        <source>No set has been imported.</source>
        <translation>아무 확장판도 불러오지 못했습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="488"/>
        <source>Sets imported</source>
        <translation>확장판 불러오기 완료</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="489"/>
        <source>The following sets has been imported. Press &quot;Save&quot; to save the imported cards to the Cockatrice database.</source>
        <translation>아래와 같이 확장판을 불러왔습니다.
&quot;저장&quot; 버튼을 눌러 코카트리스에서 사용할 수 있는 카드 데이터베이스를 저장하실 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="492"/>
        <source>Save to the default path (recommended)</source>
        <translation>기본 경로에 저장 (권장)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="501"/>
        <source>Import finished: %1 cards.</source>
        <translation>총 %1장의 카드 불러오기 완료</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="503"/>
        <source>%1: %2 cards imported</source>
        <translation>%1에서 %2장의 카드 불러옴</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="512"/>
        <source>Save card database</source>
        <translation>카드 데이터베이스 저장</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="513"/>
        <source>XML; card database (*.xml)</source>
        <translation>카드 데이터베이스 XML 파일 (*.xml)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="534"/>
        <source>Success</source>
        <translation>성공</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="535"/>
        <source>The card database has been saved successfully to
%1</source>
        <translation>카드 데이터베이스를 다음 위치에 저장했습니다:
%1</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="537"/>
        <source>The file could not be saved to %1</source>
        <translation>파일을 %1에 저장 할 수 없습니다.</translation>
    </message>
</context>
<context>
    <name>SaveTokensPage</name>
    <message>
        <location filename="../src/oraclewizard.cpp" line="698"/>
        <source>Tokens imported</source>
        <translation>토큰 불러오기 완료</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="699"/>
        <source>The tokens has been imported. Press &quot;Save&quot; to save the imported tokens to the Cockatrice tokens database.</source>
        <translation>토큰 파일을 불러왔습니다.
&quot;저장&quot; 버튼을 누르면 코카트리스에서 토큰 파일을 불러옵니다.</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="702"/>
        <source>Save to the default path (recommended)</source>
        <translation>기본 경로에 저장 (권장)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="709"/>
        <source>Save token database</source>
        <translation>토큰 파일 저장</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="710"/>
        <source>XML; token database (*.xml)</source>
        <translation>토큰 정보 XML 파일 (*.xml)</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="731"/>
        <source>Success</source>
        <translation>성공</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="732"/>
        <source>The token database has been saved successfully to
%1</source>
        <translation>토큰 정보 파일을 다음 위치에 저장했습니다:
%1</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="734"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../src/oraclewizard.cpp" line="734"/>
        <source>The file could not be saved to %1</source>
        <translation>파일을 %1에 저장 할 수 없습니다.</translation>
    </message>
</context>
<context>
    <name>UnZip</name>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1180"/>
        <source>ZIP operation completed successfully.</source>
        <translation>압축파일 작업을 성공적으로 완료하였습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1181"/>
        <source>Failed to initialize or load zlib library.</source>
        <translation>zlib 라이브러리를 초기화하거나 불러올 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1182"/>
        <source>zlib library error.</source>
        <translation>zlib 라이브러리 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1183"/>
        <source>Unable to create or open file.</source>
        <translation>파일을 만들거나 열 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1184"/>
        <source>Partially corrupted archive. Some files might be extracted.</source>
        <translation>압축파일의 일부가 손상되었습니다. 몇몇 파일은 압축이 풀렸을 수도 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1185"/>
        <source>Corrupted archive.</source>
        <translation>압축파일이 손상되었습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1186"/>
        <source>Wrong password.</source>
        <translation>비밀번호가 틀렸습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1187"/>
        <source>No archive has been created yet.</source>
        <translation>압축파일이 아직 생성되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1188"/>
        <source>File or directory does not exist.</source>
        <translation>파일이나 디렉토리가 존재하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1189"/>
        <source>File read error.</source>
        <translation>파일 읽기 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1190"/>
        <source>File write error.</source>
        <translation>파일 쓰기 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1191"/>
        <source>File seek error.</source>
        <translation>파일 찾기 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1192"/>
        <source>Unable to create a directory.</source>
        <translation>디렉토리를 생성할 수 없었습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1193"/>
        <source>Invalid device.</source>
        <translation>잘못된 장치입니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1194"/>
        <source>Invalid or incompatible zip archive.</source>
        <translation>잘못되거나 지원하지 않는 압축파일입니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1195"/>
        <source>Inconsistent headers. Archive might be corrupted.</source>
        <translation>헤더가 손상되었습니다. 압축 파일이 손상됐을 가능성이 있습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/unzip.cpp" line="1199"/>
        <source>Unknown error.</source>
        <translation>알 수 없는 오류.</translation>
    </message>
</context>
<context>
    <name>Zip</name>
    <message>
        <location filename="../src/zip/zip.cpp" line="1604"/>
        <source>ZIP operation completed successfully.</source>
        <translation>압축파일 작업을 성공적으로 완료하였습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1605"/>
        <source>Failed to initialize or load zlib library.</source>
        <translation>zlib 라이브러리를 초기화하거나 불러올 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1606"/>
        <source>zlib library error.</source>
        <translation>zlib 라이브러리 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1607"/>
        <source>Unable to create or open file.</source>
        <translation>파일을 만들거나 열 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1608"/>
        <source>No archive has been created yet.</source>
        <translation>압축파일이 아직 생성되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1609"/>
        <source>File or directory does not exist.</source>
        <translation>파일이나 디렉토리가 존재하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1610"/>
        <source>File read error.</source>
        <translation>파일 읽기 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1611"/>
        <source>File write error.</source>
        <translation>파일 쓰기 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1612"/>
        <source>File seek error.</source>
        <translation>파일 찾기 오류.</translation>
    </message>
    <message>
        <location filename="../src/zip/zip.cpp" line="1616"/>
        <source>Unknown error.</source>
        <translation>알 수 없는 오류.</translation>
    </message>
</context>
<context>
    <name>i18n</name>
    <message>
        <location filename="../../cockatrice/src/settingscache.cpp" line="147"/>
        <source>English</source>
        <translation>한국어 (Korean)</translation>
    </message>
</context>
</TS>