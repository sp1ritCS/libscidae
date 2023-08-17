#include "scidaedataid.h"
#include <sys/random.h>

_Static_assert(sizeof(ScidaeDataId) == sizeof(unsigned __int128), "ScidaeDataId size");
G_DEFINE_BOXED_TYPE(ScidaeDataId, scidae_data_id, scidae_data_id_copy, scidae_data_id_free)

#define BYTELEN 8

ScidaeDataId* scidae_data_id_new(void) {
	ScidaeDataId* new = g_new(ScidaeDataId, 1);
	new->timestamp = time(NULL);
	new->randomness = ((guint64)g_random_int() << (sizeof(guint32))*BYTELEN) | g_random_int();
	return new;
}

ScidaeDataId* scidae_data_id_copy(ScidaeDataId* self) {
	ScidaeDataId* new = g_new(ScidaeDataId, 1);
	new->timestamp = self->timestamp;
	new->randomness = self->randomness;
	return new;
}

void scidae_data_id_free(ScidaeDataId* self) {
	g_free(self);
}
