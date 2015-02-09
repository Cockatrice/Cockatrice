<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ko_KR">
<context>
    <name>ChooseSetsPage</name>
    <message>
        <source>Sets selection</source>
        <translation>판본 선택</translation>
    </message>
    <message>
        <source>The following sets has been found in the source file. Please mark the sets that will be imported.
All core and expansion sets are selected by default.</source>
        <translation>아래 목록은 불러올 수 있는 판본의 목록입니다.
코카트리스에서 사용 할 판본들을 선택하여 주세요.
모든 기본판과 확장판은 기본으로 선택되어 있습니다. </translation>
    </message>
    <message>
        <source>&amp;Check all</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <source>&amp;Uncheck all</source>
        <translation>모두 선택 해제</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <source>Please mark at least one set.</source>
        <translation>최소 한개 이상의 판본을 선택해주세요.</translation>
    </message>
</context>
<context>
    <name>IntroPage</name>
    <message>
        <source>Introduction</source>
        <translation>개요</translation>
    </message>
    <message>
        <source>English</source>
        <translation>한국어</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>언어</translation>
    </message>
    <message>
        <source>This wizard will import the list of sets and cards that will be used by Cockatrice.&lt;br/&gt;You will need to specify an url or a filename that will be used as a source, and then choose the wanted sets from the list of the available ones.</source>
        <translation>오라클은 코카트리스에서 사용할 판본의 목록과 카드들의 정보를 갱신하는 프로그램입니다.&lt;br/&gt;판본 목록 파일이 있는 웹 주소나 파일을 입력하신 후에 읽어온 목록에서 원하는 판본을 선택하여 불러올 수 있습니다.</translation>
    </message>
</context>
<context>
    <name>LoadSetsPage</name>
    <message>
        <source>Source selection</source>
        <translation>판본 목록 파일 주소 입력</translation>
    </message>
    <message>
        <source>Please specify a source for the list of sets and cards. You can specify an url address that will be download or use an existing file from your computer.</source>
        <translation>판본 목록 및 카드 정보가 들어있는 파일의 위치를 입력하여 주십시오. 
다운로드 할 수 있는 웹 주소나 컴퓨터에 저장되어 있는 파일을 선택 할 수 있습니다.</translation>
    </message>
    <message>
        <source>Download url:</source>
        <translation>웹 주소:</translation>
    </message>
    <message>
        <source>Local file:</source>
        <translation>파일 위치:</translation>
    </message>
    <message>
        <source>Restore default url</source>
        <translation>기본 주소로 복원</translation>
    </message>
    <message>
        <source>Choose file...</source>
        <translation>파일 선택...</translation>
    </message>
    <message>
        <source>Load sets file</source>
        <translation>판본 목록 파일 불러오기</translation>
    </message>
    <message>
        <source>Sets JSON file (*.json *.zip)</source>
        <translation>판본 목록 JSON 파일 (*.json *.zip)</translation>
    </message>
    <message>
        <source>Sets JSON file (*.json)</source>
        <translation>판본 목록 JSON 파일 (*.json)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <source>The provided url is not valid.</source>
        <translation>잘못된 주소를 입력하셨습니다.</translation>
    </message>
    <message>
        <source>Downloading (0MB)</source>
        <translation>다운로드 중 (0MB)</translation>
    </message>
    <message>
        <source>Please choose a file.</source>
        <translation>판본 목록 파일을 선택하여 주세요.</translation>
    </message>
    <message>
        <source>Cannot open file &apos;%1&apos;.</source>
        <translation>파일 &apos;%1&apos;을(를) 열 수 없습니다.</translation>
    </message>
    <message>
        <source>Downloading (%1MB)</source>
        <translation>다운로드 중 (%1MB)</translation>
    </message>
    <message>
        <source>Network error: %1.</source>
        <translation>네트워크 오류 : %1.</translation>
    </message>
    <message>
        <source>Parsing file</source>
        <translation>목록 파싱중</translation>
    </message>
    <message>
        <source>Failed to open Zip archive: %1.</source>
        <translation>압축파일 열기 실패 : %1.</translation>
    </message>
    <message>
        <source>Zip extraction failed: the Zip archive doesn&apos;t contain exactly one file.</source>
        <translation>압축 풀기 실패 : 압축 파일에 판본 목록 파일 이외의 파일이 있습니다.</translation>
    </message>
    <message>
        <source>Zip extraction failed: %1.</source>
        <translation>압축 풀기 실패 : %1.</translation>
    </message>
    <message>
        <source>Sorry, this version of Oracle does not support zipped files.</source>
        <translation>죄송합니다. 본 버전에서는 압축 파일을 지원하지 않습니다.</translation>
    </message>
    <message>
        <source>Do you want to try to download a fresh copy of the uncompressed file instead?</source>
        <translation>압축되지 않은 판본 목록을 대신 내려받으시겠습니까?</translation>
    </message>
    <message>
        <source>The file was retrieved successfully, but it does not contain any sets data.</source>
        <translation>파일을 성공적으로 다운로드 하였으나 판본 목록 정보가 없습니다.</translation>
    </message>
</context>
<context>
    <name>OracleImporter</name>
    <message>
        <source>Dummy set containing tokens</source>
        <translation>토큰 정보가 들어있는 더미 판본</translation>
    </message>
</context>
<context>
    <name>OracleWizard</name>
    <message>
        <source>Oracle Importer</source>
        <translation>오라클</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>저장</translation>
    </message>
</context>
<context>
    <name>SaveSetsPage</name>
    <message>
        <source>Sets imported</source>
        <translation>판본 불러오기 완료</translation>
    </message>
    <message>
        <source>The following sets has been imported. Press &quot;Save&quot; to save the imported cards to the Cockatrice database.</source>
        <translation>아래와 같이 판본을 불러왔습니다.
&quot;저장&quot; 버튼을 눌러 코카트리스에서 사용할 수 있는 카드 데이터베이스를 저장하실 수 있습니다.</translation>
    </message>
    <message>
        <source>Save to the default path (recommended)</source>
        <translation>기본 경로에 저장 (권장)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <source>No set has been imported.</source>
        <translation>아무 판본도 불러오지 못했습니다.</translation>
    </message>
    <message>
        <source>Import finished: %1 cards.</source>
        <translation>총 %1장의 카드 불러오기 완료</translation>
    </message>
    <message>
        <source>%1: %2 cards imported</source>
        <translation>%1에서 %2장의 카드 불러옴</translation>
    </message>
    <message>
        <source>Save card database</source>
        <translation>카드 데이터베이스 저장</translation>
    </message>
    <message>
        <source>XML; card database (*.xml)</source>
        <translation>카드 데이터베이스 XML 파일 (*.xml)</translation>
    </message>
    <message>
        <source>Success</source>
        <translation>성공</translation>
    </message>
    <message>
        <source>The card database has been saved successfully to
%1</source>
        <translation>카드 데이터베이스를 다음 위치에 저장했습니다:
%1</translation>
    </message>
    <message>
        <source>The file could not be saved to %1</source>
        <translation>파일을 %1에 저장 할 수 없습니다.</translation>
    </message>
</context>
<context>
    <name>UnZip</name>
    <message>
        <source>ZIP operation completed successfully.</source>
        <translation>압축파일 작업을 성공적으로 완료하였습니다.</translation>
    </message>
    <message>
        <source>Failed to initialize or load zlib library.</source>
        <translation>zlib 라이브러리를 초기화하거나 불러올 수 없습니다.</translation>
    </message>
    <message>
        <source>zlib library error.</source>
        <translation>zlib 라이브러리 오류.</translation>
    </message>
    <message>
        <source>Unable to create or open file.</source>
        <translation>파일을 만들거나 열 수 없습니다.</translation>
    </message>
    <message>
        <source>Partially corrupted archive. Some files might be extracted.</source>
        <translation>압축파일의 일부가 손상되었습니다. 몇몇 파일은 압축이 풀렸을 수도 있습니다.</translation>
    </message>
    <message>
        <source>Corrupted archive.</source>
        <translation>압축파일이 손상되었습니다.</translation>
    </message>
    <message>
        <source>Wrong password.</source>
        <translation>비밀번호가 틀렸습니다.</translation>
    </message>
    <message>
        <source>No archive has been created yet.</source>
        <translation>압축파일이 아직 생성되지 않았습니다.</translation>
    </message>
    <message>
        <source>File or directory does not exist.</source>
        <translation>파일이나 디렉토리가 존재하지 않습니다.</translation>
    </message>
    <message>
        <source>File read error.</source>
        <translation>파일 읽기 오류.</translation>
    </message>
    <message>
        <source>File write error.</source>
        <translation>파일 쓰기 오류.</translation>
    </message>
    <message>
        <source>File seek error.</source>
        <translation>파일 찾기 오류.</translation>
    </message>
    <message>
        <source>Unable to create a directory.</source>
        <translation>디렉토리를 생성할 수 없었습니다.</translation>
    </message>
    <message>
        <source>Invalid device.</source>
        <translation>잘못된 장치입니다.</translation>
    </message>
    <message>
        <source>Invalid or incompatible zip archive.</source>
        <translation>잘못되거나 지원하지 않는 압축파일입니다.</translation>
    </message>
    <message>
        <source>Inconsistent headers. Archive might be corrupted.</source>
        <translation>헤더가 손상되었습니다. 압축 파일 자체의 손상도 가능합니다.</translation>
    </message>
    <message>
        <source>Unknown error.</source>
        <translation>알 수 없는 오류.</translation>
    </message>
</context>
<context>
    <name>Zip</name>
    <message>
        <source>ZIP operation completed successfully.</source>
        <translation>압축파일 작업을 성공적으로 완료하였습니다.</translation>
    </message>
    <message>
        <source>Failed to initialize or load zlib library.</source>
        <translation>zlib 라이브러리를 초기화하거나 불러올 수 없습니다.</translation>
    </message>
    <message>
        <source>zlib library error.</source>
        <translation>zlib 라이브러리 오류.</translation>
    </message>
    <message>
        <source>Unable to create or open file.</source>
        <translation>파일을 만들거나 열 수 없습니다.</translation>
    </message>
    <message>
        <source>No archive has been created yet.</source>
        <translation>압축파일이 아직 생성되지 않았습니다.</translation>
    </message>
    <message>
        <source>File or directory does not exist.</source>
        <translation>파일이나 디렉토리가 존재하지 않습니다.</translation>
    </message>
    <message>
        <source>File read error.</source>
        <translation>파일 읽기 오류.</translation>
    </message>
    <message>
        <source>File write error.</source>
        <translation>파일 쓰기 오류.</translation>
    </message>
    <message>
        <source>File seek error.</source>
        <translation>파일 찾기 오류.</translation>
    </message>
    <message>
        <source>Unknown error.</source>
        <translation>알 수 없는 오류.</translation>
    </message>
</context>
</TS>
