#include "ssu_find.h"

void list(Set *set, bool sort_set, bool c_opt[5], bool sort_up){
    // 파일 크기 순 정렬일 경우 그대로 출력 -> 리스트는 크기 같으므로 경로 순 출력
    if(c_opt[1]){
        int set_size = get_setLen(set);

        if(sort_up) sort_upSet(set, set_size); // 오름차순
        else sort_downSet(set, set_size); // 내림차순
        
        print_set(set);
    }
    // 파일 절대 경로 순인 경우
    else if(c_opt[0]){

    }


}

void list_fileset();

void list_filelist();

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