# SilencePNG

Windows 환경에서 `oxipng`를 사용해 PNG 이미지를 무손실 압축 및 최적화하는 CLI 도구.

## 주요 기능

- **재귀 탐색**: 지정한 디렉토리 및 하위 디렉토리의 모든 `.png` 파일 탐색
- **단일 파일 지원**: 단일 PNG 파일 경로 전달 시 해당 파일만 압축
- **의존성 자동 설치**: 시스템에 `oxipng`가 없으면 `winget`을 통해 자동 설치
- **작업 취소 처리**: `Ctrl + C` 입력 시 하위 프로세스를 즉시 종료하고 안전하게 중단
- **결과 통계**: 최적화 전/후 크기, 절감 용량, 수정된 파일 수 출력

## 요구 사항

- Windows (x64)
- [oxipng](https://github.com/shssoichiro/oxipng) (미설치 시 최초 실행 시 winget으로 자동 설치)

## 사용법

```cmd
silencepng <파일 또는 폴더 경로>
```

### 실행 예시

폴더 내 모든 PNG 최적화:
```cmd
silencepng "C:\path\to\images"
```

단일 PNG 최적화:
```cmd
silencepng "C:\path\to\image.png"
```

## 빌드 방법

### GCC (MinGW-w64)
```cmd
gcc -O2 -s silencepng.c -o silencepng.exe
```

### MSVC
```cmd
cl /O2 /Fe:silencepng.exe silencepng.c
```

## 설치 패키지 (Inno Setup)

[Inno Setup](https://jrsoftware.org/isdl.php)을 통해 `SilencePNG.iss`를 컴파일하면 설치 프로그램(`SilencePNG-Setup.exe`)이 생성됩니다.
- 설치 시 시스템 `PATH` 환경 변수에 자동으로 설치 경로가 등록됩니다.
