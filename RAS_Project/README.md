1. 리눅스 API 교육 과정의 최종 프로젝트로 진행한 REST API Server에 다음 기능을 추가
- Telcobase DB에 연동 허용할 Client IP를 관리하고, Client 연결 요청 시, 허용 Client IP가 아닌 경우 즉시 Client 연결 해제
- 허용할 Client IP에 대한 등록/삭제/조회 MMC 구현
- POST/GET/DELETE Request/Response 송수신 개수에 대한 통계 생성. Response 통계는 응답 코드별로 통계 생성.(서버에서 사용하는 응답 코드에 대해서만 생성하면 됨)
- Client IP로 Trace 가능하도록 Trace 등록/삭제/조회 MMC 구현 및 Trace 기능 구현. Trace에 출력되는 정보는 HTTP Request/Response 모두 나와야 함
- 사용자 정보는 디렉토리/파일이 아니라 Telcobase DB에 저장하도록 구현하고 사용자 정보 조회/삭제를 위한 MMC 구현
- Telcobase에 저장된 사용자 수에 따른 알람 기능 구현
 * 0  ~ 10: Normal
 * 11 ~ 20: Minor 
 * 21 ~ 30: Major
 * 31 ~   : Critical
- Server가 listen할 IP/Port 정보는 config 파일에서 프로그램 기동시 읽도록 기능 추가

2. 프로젝트 진행
- 일정은 본인이 수립 - 설계/구현/자체테스트/발표
- 설계 끝나면 설계 문서 가지고 설계 회의 진행
- 구현은 주단위로 소스 리뷰 진행
- 구현 완료후 Coverity로 defect 체크하여 detect 없도록 수정
- 프로젝트 관련 산출물(설계 문서(블록 설계서, MMC 설명서, 통계 설명서, 환경 파일 설명서), 자체 시험 항목서, 자체 시험 결과서 등)은 Confluence 직접 작성
- 팀 코딩 가이드를 준수하여 코딩
