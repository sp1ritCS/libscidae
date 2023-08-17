#include <gtk/gtk.h>
#include <scidae.h>

static void activate(GtkApplication* app, gpointer) {
	GtkWidget* window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "libscidae demo");
	
	GtkWidget* scroll = gtk_scrolled_window_new();

	g_autoptr(Pango2Context) pango_ctx = pango2_context_new();
	g_autoptr(ScidaeContext) ctx = scidae_pango_context_new(pango_ctx);
	
	ScidaeWidget* para = scidae_paragraph_new(ctx);
	
	ScidaeText* w_alpha = scidae_context_create_text_widget(ctx, "Hello\nNA");
	ScidaeText* w_beta = scidae_context_create_text_widget(ctx, "Hello");
	ScidaeText* w_gamma = scidae_context_create_text_widget(ctx, "Lorem ipsum dolor sit amet. This is just sample text to see if wrapping works as intended!");
	ScidaeText* w_delta = scidae_context_create_text_widget(ctx, "\nLine\nBreak\nLorem ipsum\nDolor sit\n amet.\nFFF\nasd\ngam\nsd\nmore\nwrapping until this works as I want ot to work\nhave we\n understood us in that regard?\nEnter Yes or No");
	scidae_paragraph_add_child(SCIDAE_PARAGRAPH(para), SCIDAE_WIDGET(w_alpha));
	scidae_paragraph_add_child(SCIDAE_PARAGRAPH(para), SCIDAE_WIDGET(w_beta));
	scidae_paragraph_add_child(SCIDAE_PARAGRAPH(para), SCIDAE_WIDGET(w_gamma));
	scidae_paragraph_add_child(SCIDAE_PARAGRAPH(para), SCIDAE_WIDGET(w_delta));
	
	GtkWidget* canvas = scidae_canvas_new(SCIDAE_TOPLEVEL(para));
	
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), canvas);

	gtk_window_set_child(GTK_WINDOW(window), scroll);
	gtk_window_present(GTK_WINDOW(window));
}

#include <scidaedata.h>
static void test_data(void) {
	g_autoptr(GPtrArray) revisions_first = g_ptr_array_new();
	g_ptr_array_add(revisions_first, scidae_revision_insert_new(NULL, "Hello ", 0));
	g_autoptr(ScidaeWordNode) first = scidae_word_node_new(revisions_first);

	g_autoptr(GPtrArray) revisions_second = g_ptr_array_new();
	g_ptr_array_add(revisions_second, scidae_revision_insert_new(NULL, "World!", 0));
	g_ptr_array_add(revisions_second, scidae_revision_delete_new(NULL, 0, 5));
	g_ptr_array_add(revisions_second, scidae_revision_insert_new(NULL, "Scidae", 0));
	g_autoptr(ScidaeWordNode) second = scidae_word_node_new(revisions_second);

	scidae_word_node_set_next(first, second);
	scidae_word_node_set_prev(second, first);

	gchar* o = g_strconcat(scidae_word_node_get_string(first), scidae_word_node_get_string(second), NULL);
	g_print("Buf: %s\n", o);
	g_free(o);
}

int main(int argc, char** argv) {
	test_data();

	g_autoptr(GtkApplication) app = gtk_application_new("arpa.sp1rit.scidae.demov2", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}
