디버깅 가이드 — CodeLLDB로 `test/test-rbtree` 디버깅

요약
- F5로 `Debug test-rbtree (CodeLLDB)` 구성 실행.
- 빌드(preLaunchTask): `build-test`가 `make -C test test`를 실행합니다.
- launch.json에서 LLDB initCommands로 자동 설정된 중단점: `rbtree_erase`, `delete_rbtree`, `free_subtree`, `new_node`.
- ASAN 옵션: `ASAN_OPTIONS=detect_leaks=0:verbosity=1:halt_on_error=1` (에러 발생 시 디버거에서 즉시 정지).

중단점에서 확인할 항목
- `rbtree_erase` 직전/직후:
  - 인자: `p` (삭제 요청 노드). 확인: `frame variable p` 또는 `frame variable *p`.
  - `target`과 `child` (함수 내부 변수). 확인: `frame variable target child`.
  - `target->parent`, `target->left`, `target->right`, `target->color`.
  - `t->root` 와 `t->nil` 주소 일관성 확인: `frame variable t` 또는 `frame variable t->root t->nil`.
  - `free(target)` 직전에 같은 주소가 이미 해제되었는지(ASan 메시지 참조).

- `free_subtree` 진입 시:
  - 인자 `p`(현재 노드 포인터). 확인: `frame variable p` 및 `frame variable p->left p->right`.
  - 동일 주소의 중복 해제가 발생하는지: `p`의 주소와 과거에 freed 된 주소를 비교.

권장 LLDB 명령 모음 (Debug Console 또는 LLDB 콘솔 입력)
- 중단점 설정(수동):
  breakpoint set --name rbtree_erase
  breakpoint set --name delete_rbtree
  breakpoint set --name free_subtree
  breakpoint set --name new_node

- 실행/제어
  run
  continue
  step
  next
  finish

- 상태 출력
  bt                            # 백트레이스
  frame variable p target child  # 현재 프레임 변수 출력
  frame variable t->root t->nil  # 트리 루트/센티넬
  frame variable p->parent p->left p->right p->color p->key
  expr (int) p->key              # 표현식 평가 (형변환으로 출력 형 맞춤)
  p/x (void*)p                   # 포인터 주소를 16진수로 확인

- 메모리/워치
  memory read --format "pointer" --count 8 <addr>  # 메모리에서 포인터 블록 읽기
  watchpoint set variable p      # 변수 변경 감시(워치포인트) — 비용 큼

디버깅 체크리스트 (우선순위)
1) `rbtree_erase`에서 `target`이 올바르게 결정되었는지 확인. (2자식인 경우 successor가 target으로 바뀌는 로직)
2) `target` 제거 시 부모의 자식 포인터와 `child->parent`가 적절히 갱신되는지 확인.
3) `free(target)` 직전 `target` 주소가 이미 freed 됐는지(ASan 메시지와 비교).
4) `free_subtree` 진입 시 `p`가 `t->nil`이 아닌지, 동일 주소를 두 번 방문하지 않는지 확인.

추가 팁
- breakpoint에 조건을 붙여 특정 키 값에서만 멈추게 할 수 있습니다:
  breakpoint modify --condition "p->key == 24"
- LLDB 콘솔에서 `frame variable` 출력이 불충분하면 `expr`로 포인터나 필드를 직접 출력하세요.

문제 재현
- 터미널에서 직접 실행(빌드+테스트):
```sh
make -C test test
```
- 빌드/실행 로그는 `/tmp/rbtree_latest.log`에 저장되어 있을 수 있습니다 (이 세션에서 캡처된 파일).

문의/원하면 제가 도와드릴 항목
- 특정 중단점에서 출력할 `frame variable` 명령을 더 정교하게 만들어 드립니다.
- 자동화된 LLDB 명령 스크립트(`.lldbinit`)로 더 상세한 검사 스크립트를 만들어 드립니다.

빠른 시작
1) VS Code에서 좌측 Debug(디버그) 패널 열기
2) 구성 `Debug test-rbtree (CodeLLDB)` 선택
3) F5 누르기
4) 중단되면 Debug Console 또는 LLDB 콘솔에서 위 명령 사용

