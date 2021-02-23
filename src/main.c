#undef __cplusplus

#include "dadnme.h"

int main(int argc, char* argv[]) {
  setbuf(stdout, NULL);

#ifdef CHECK_MEMORY_LEAKS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  struct DadNMe dad_n_me = {0};
  int dad_n_me_error = dad_n_me_init(&dad_n_me);
  switch (dad_n_me_error) {
    case (DAD_N_ME_SUCCESS):
      break;
    case (DAD_N_ME_MANA_ERROR):
      fprintf(stderr, "Failed to setup Dad n' Me!\n");
      return 1;
    default:
      fprintf(stderr, "Unknown Dad n' Me error! Error code: %d\n", dad_n_me_error);
      return 2;
  }
  dad_n_me_start(&dad_n_me);
  dad_n_me_delete(&dad_n_me);

  return 0;
}
