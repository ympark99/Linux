#include "ssu_find.h"

void list(Set *set, bool sort_set, bool c_opt[5], bool sort_up){
    // 파일 크기 순 정렬일 경우 그대로 출력 -> 리스트는 크기 같으므로 경로 순 출력
    if(c_opt[1]){
        int set_size = get_setLen(set);
        if(sort_up) sort_upSet(set, set_size); // 오름차순
        else sort_downSet(set, set_size); // 내림차순
    }
    // 파일 절대 경로 순인 경우
    else if(c_opt[0]){
        Set *cur = set->next;
        int list_size;
        while(cur != NULL){
            list_size = get_listLen(cur->nodeList);
            sort_pathNode(cur->nodeList, list_size, sort_up);
            cur = cur->next;
        }
    }
    // uid / gid / mode 기준 정렬, 세트 정렬은 관련 없음
    else if(c_opt[2] || c_opt[3] || c_opt[4]){
        // 세트정렬은 관련없으므로 그대로
        // 리스트정렬
        if(sort_up){
            Set *cur = set->next;
            int list_size;
            while(cur != NULL){
                list_size = get_listLen(cur->nodeList);

                if(c_opt[2]) sort_idUpList(cur->nodeList, list_size, 0);
                else if(c_opt[3]) sort_idUpList(cur->nodeList, list_size, 1);
                else if(c_opt[4]) sort_idUpList(cur->nodeList, list_size, 2);

                cur = cur->next;
            }
        }
        else{ // 내림차순
            Set *cur = set->next;
            int list_size;
            while(cur != NULL){
                list_size = get_listLen(cur->nodeList);

                if(c_opt[2]) sort_idDownList(cur->nodeList, list_size, 0);
                else if(c_opt[3]) sort_idDownList(cur->nodeList, list_size, 1);
                else if(c_opt[4]) sort_idDownList(cur->nodeList, list_size, 2);

                cur = cur->next;
            }
        }
    }
    print_set(set); // 세트 출력
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

// filename, 절대경로 정렬
void sort_pathNode(Node *list, int list_size, bool sort_up){
    Node *cur = list->next; // head 다음
	int get_longPath = -1;
	int cmp_res = 0;

    for (int i = 0; i < list_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < list_size - 1 - i; j++){
			char* name1 = strrchr(cur->path, '/'); // 파일 이름 추출
			char* name2 = strrchr(cur->next->path, '/'); // 파일 이름 추출
			cmp_res = strcmp(name1, name2);
            if((cmp_res > 0) && sort_up)
                swap_node(cur, cur->next); // swap
            else if((cmp_res < 0) && !sort_up)
                swap_node(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = list->next;
    }
}

// uid / gid 기준 오름차순 정렬
// sortWhat : 0->uid, 1->gid, 2->mode
void sort_idUpList(Node *list, int list_size, int sortWhat){
    Node *cur = list->next; // head 다음
    for (int i = 0; i < list_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < list_size - 1 - i; j++){
            if((cur->uid > cur->next->uid) && (sortWhat == 0))
                swap_node(cur, cur->next); // swap
            else if((cur->gid > cur->next->gid) && (sortWhat == 1))
                swap_node(cur, cur->next); // swap
            else if((cur->mode > cur->next->mode) && (sortWhat == 2))
                swap_node(cur, cur->next); // swap                
            cur = cur->next;
        }
        cur = list->next;
    }
}

// uid / gid 기준 오름차순 정렬
void sort_idDownList(Node *list, int list_size, int sortWhat){
    Node *cur = list->next; // head 다음
    for (int i = 0; i < list_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < list_size - 1 - i; j++){
            if((cur->uid < cur->next->uid) && (sortWhat == 0))
                swap_node(cur, cur->next); // swap
            else if((cur->gid < cur->next->gid) && (sortWhat == 1))
                swap_node(cur, cur->next); // swap
            else if((cur->mode < cur->next->mode) && (sortWhat == 2))
                swap_node(cur, cur->next); // swap                
            cur = cur->next;
        }
        cur = list->next;
    }
}