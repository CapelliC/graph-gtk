#include "graph-gtk-pad.h"
#include "graph-gtk-node.h"
#include "graph-gtk-connection.h"

static void graph_gtk_pad_dispose (GObject *object);
static void graph_gtk_pad_finalize (GObject *object);
static void graph_gtk_pad_default_render(GraphGtkPad* self, cairo_t* cairo);

G_DEFINE_TYPE (GraphGtkPad, graph_gtk_pad, G_TYPE_OBJECT);

static void
graph_gtk_pad_class_init (GraphGtkPadClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = graph_gtk_pad_dispose;
  gobject_class->finalize = graph_gtk_pad_finalize;

  klass->render_pad = graph_gtk_pad_default_render;
}

static void
graph_gtk_pad_init (GraphGtkPad *self)
{
  self->rel_x = 0;
  self->rel_y = 0;
  self->node = NULL;
}

static void
graph_gtk_pad_dispose (GObject *object)
{
  //CC GraphGtkPad *self = (GraphGtkPad *)object;

  G_OBJECT_CLASS (graph_gtk_pad_parent_class)->dispose (object);
}

static void
graph_gtk_pad_finalize (GObject *object)
{
  //CC GraphGtkPad *self = (GraphGtkPad *)object;

  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (graph_gtk_pad_parent_class)->finalize (object);
}


static void
graph_gtk_pad_default_render(GraphGtkPad* self, cairo_t* cr)
{
  gboolean connected = (g_list_length(self->connections) > 0);

  int x, y;
  graph_gtk_pad_get_position(self, &x, &y);
  if(self->is_output)
    {
      cairo_move_to(cr, x+5, y-5);
      cairo_line_to(cr, x-5, y-5);
      cairo_line_to(cr, x-5, y+5);
      cairo_line_to(cr, x+5, y+5);
      cairo_close_path(cr);
      if(connected)
	cairo_set_source_rgb(cr, 140.0/256.0, 132.0/256.0, 27.0/256.0);
      else
	cairo_set_source_rgb(cr, 64.0/256.0, 64.0/256.0, 64.0/256.0);
      cairo_fill(cr);

      cairo_move_to(cr, x+5, y-5);
      cairo_line_to(cr, x-5, y-5);
      cairo_line_to(cr, x-5, y+5);
      cairo_line_to(cr, x+5, y+5);
      
      cairo_set_line_width(cr, 0.7);
      cairo_set_source_rgb(cr, 132.0/256.0, 132.0/256.0, 132.0/256.0);
      cairo_stroke(cr);

      cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 13);
      
      cairo_text_extents_t extents;
      cairo_text_extents(cr, self->name, &extents);

      cairo_move_to(cr, x-10-extents.width, y-(extents.y_bearing/2));
      cairo_set_source_rgb(cr, 182.0/256.0, 182.0/256.0, 182.0/256.0);
      cairo_show_text(cr, self->name);
    }
  else //is input
    {
      cairo_move_to(cr, x+5, y-5);
      cairo_line_to(cr, x-5, y-5);
      cairo_line_to(cr, x-5, y+5);
      cairo_line_to(cr, x+5, y+5);
      cairo_close_path(cr);
      if(connected)
	cairo_set_source_rgb(cr, 140.0/256.0, 132.0/256.0, 27.0/256.0);
      else
	cairo_set_source_rgb(cr, 64.0/256.0, 64.0/256.0, 64.0/256.0);
      cairo_fill(cr);

      cairo_move_to(cr, x-5, y-5);
      cairo_line_to(cr, x+5, y-5);
      cairo_line_to(cr, x+5, y+5);
      cairo_line_to(cr, x-5, y+5);
      
      cairo_set_line_width(cr, 0.7);
      cairo_set_source_rgb(cr, 132.0/256.0, 132.0/256.0, 132.0/256.0);
      cairo_stroke(cr);

      cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 13);
      
      cairo_text_extents_t extents;
      cairo_text_extents(cr, self->name, &extents);

      cairo_move_to(cr, x+10, y-(extents.y_bearing/2));
      cairo_set_source_rgb(cr, 182.0/256.0, 182.0/256.0, 182.0/256.0);
      cairo_show_text(cr, self->name);
    }

}

GraphGtkPad*
graph_gtk_pad_new(const gchar* name, gboolean is_output)
{
  GraphGtkPad *pad = GRAPH_GTK_PAD(g_object_new(GRAPH_TYPE_GTK_PAD, NULL));
  pad->name = g_strdup(name);
  pad->is_output = is_output;
  return pad;
}

void
graph_gtk_pad_render(GraphGtkPad* self, cairo_t* cairo)
{
  g_return_if_fail(IS_GRAPH_GTK_PAD(self));

  GRAPH_GTK_PAD_GET_CLASS(self)->render_pad(self, cairo);
}

void
graph_gtk_pad_get_position(GraphGtkPad* self, int *x, int *y)
{
  g_return_if_fail(IS_GRAPH_GTK_PAD(self));

  *x = self->rel_x+self->node->x+self->node->offset_x;
  *y = self->rel_y+self->node->y+self->node->offset_y;
}

gboolean
graph_gtk_pad_is_connected_to(GraphGtkPad* self, GraphGtkPad* other)
{
  if((self->is_output && other->is_output) || (!self->is_output && !other->is_output))
    return FALSE;

  GList* list;
  for(list = self->connections; list != NULL; list = list->next)
    {
      GraphGtkConnection *connection = (GraphGtkConnection*)list->data;
      GraphGtkPad *pad;
      
      if(self->is_output)
	pad = connection->sink;
      else
	pad = connection->source;

      if(pad == other)
	return TRUE;
    }

  return FALSE;
}


gdouble
graph_gtk_pad_get_width(GraphGtkPad* self)
{
  if(self->node && self->node->view)
    {
      //TODO: cache the results and only recalculate when the name changes
      GraphGtkView *view = self->node->view;
      GtkWidget *widget = GTK_WIDGET(view);
      cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

      cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 13);
      
      cairo_text_extents_t extents;
      cairo_text_extents(cr, self->name, &extents);

      cairo_destroy(cr);

      return extents.width;
    }
  else
    {
      return 100;
    }
}

void
graph_gtk_pad_connect_to(GraphGtkPad* source, GraphGtkPad* sink)
{
  if(!source->is_output || sink->is_output)
    {
      g_warning("graph_gtk_pad_connect_to(): can only connect from an output pad to an input pad\n");
      return;
    }

  if(!graph_gtk_pad_is_connected_to(source, sink))
    {
      GraphGtkConnection *connection = graph_gtk_connection_new(source, sink);

      source->connections = g_list_append(source->connections, connection);

      graph_gtk_pad_disconnect(sink);
      sink->connections = g_list_append(sink->connections, connection);
    }
}

//Remove all connections from this pad
void
graph_gtk_pad_disconnect(GraphGtkPad* self)
{
  GList* list;
  for(list = self->connections; list != NULL; list = list->next)
    {
      GraphGtkConnection *connection = (GraphGtkConnection*)list->data;

      GraphGtkPad *other;
      
      if(self->is_output)
	{
	  other = connection->sink;
	  g_signal_emit_by_name(self->node->view, "nodes-disconnected", 
				self->node, self->name,
				other->node, other->name);
	}
      else
	{
	  other = connection->source;
	  g_signal_emit_by_name(self->node->view, "nodes-disconnected", 
				other->node, other->name,
				self->node, self->name);
	}

      other->connections = g_list_remove(other->connections, connection);

      g_object_unref(connection);
    }

  g_list_free(self->connections);
  self->connections = NULL;
}
