#ifndef __SCIDAEDATAID_H__
#define __SCIDAEDATAID_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_DATA_ID (scidae_data_id_get_type())

struct _ScidaeDataId {
	guint64 timestamp;
	guint64 randomness;
} __attribute__((packed));
typedef struct _ScidaeDataId ScidaeDataId;

GType scidae_data_id_get_type(void);

ScidaeDataId* scidae_data_id_new(void);

ScidaeDataId* scidae_data_id_copy(ScidaeDataId* self);

void scidae_data_id_free(ScidaeDataId* self);

G_END_DECLS

#endif // __SCIDAEDATAID_H__
