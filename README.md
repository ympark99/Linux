## 리눅스시스템프로그래밍 설계

- 리눅스 배포 환경에서의 프로그램 구현 -> 리눅스 배포 환경에서 사용가능 (ubuntu, mac os, ... etc)
- 총 3개의 설계를 구현하였으며 각 설계의 정보는 폴더 내의 readme 참고

- p1(ssu_sindex) : 주어진 디렉토리 내 특정 파일(이하 정규 파일과 디렉토리 파일 모두 가능)과 이름 및 크기가 동일한 정규 파일(이하 정규 파일과 디렉토리 파일 모두 가능)을 찾고 비교하는 프로그램
- p2(ssu_sdup) : 시스템 내 존재하는 동일(중복)한 파일을 찾고 삭제하는 프로그램
- p3(ssu_sfinder) : 지정한 레드 개수만큼 리눅스 시스템 내 존재하는 동일한(중복) 파일을 찾고 삭제 및 복원하는 ssu_sfinder 프로그램
