#include "ssu_find.h"

bool isFilePathUp = true; // 현재 파일 절대경로 오름차순인지 확인

void list(Set *set, bool sort_set, bool c_opt[5], bool sort_up){
    // 파일 크기 순 정렬일 경우 그대로 출력 -> 리스트는 크기 같으므로 경로 순 출력
    if(c_opt[1]){
        int set_size = get_setLen(set);

        if(sort_up) sort_upSet(set, set_size); // 오름차순
        else sort_downSet(set, set_size); // 내림차순
        
        print_set(set); // 세트 출력
    }
    // 파일 절대 경로 순인 경우
    else if(c_opt[0]){
        printf("isfilepathup : %d\n", isFilePathUp);
        // 세트정렬은 관련없으므로 그대로
        // 리스트정렬 : 가장 처음 절대경로 오름차순 -> 반대의 명령 왔을때 뒤집기
        if((sort_up && !isFilePathUp) || (!sort_up && isFilePathUp)){
            Set *cur = set->next;
            int list_size;
            while(cur != NULL){
                list_size = get_listLen(cur->nodeList);
                sort_pathReverse(cur->nodeList, list_size); // 리스트 뒤집기
                cur = cur->next;
            }
            isFilePathUp = !isFilePathUp; // 현재 상태 반대로
        }
        print_set(set); // 세트 출력
    }


}

// 세트 버블정렬
// 파일크기순 정렬 (bfs이므로 파일크기 같을 경우 절대경로 짧은 순 -> 임의(아스키 코드 순))
void sort_downSet(Set *set, int set_size){
    Set *cur = set->next; // head 다음
    for (int i = 0; i < set_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < set_size - 1 - i; j++){
            if(cur->filesize < cur->next->filesize)
                swap_set(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = set->next;
    }
}

// 리스트 뒤집기 (bfs이므로 처음에 절대경로 짧은 순)
void sort_pathReverse(Node *list, int list_size){
    Node *cur = list->next; // head 다음
    for (int i = 0; i < list_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < list_size - 1 - i; j++){
                swap_node(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = list->next;
    }
}