typedef struct{
  as_predexp_base ** array;
  int capacity;
  int size;
} as_predexp_array;

void init_as_predexp_array(as_predexp_array *a, int initialSize){
  a->array = (as_predexp_base **) cf_calloc(initialSize, sizeof(as_bin_name));
  a->capacity = 0;
  a->size = initialSize;
}

void insert_as_predexp_array(as_predexp_array *a, as_predexp_base * element) {
  if (a->capacity == a->size) {
    a->size *= 2;
    a->array = (as_predexp_base **) cf_realloc(a->array, a->size * sizeof(as_bin_name));
  }
  a->array[a->capacity++] = element;
}

void free_as_predexp_array(as_predexp_array *a) {
  free(a->array);
  a->array = NULL;
  a->capacity = a->size = 0;
}
