% PLGI case test: running graph-gtk, a third party widget
% translation of ~/graph-gtk/tests/simple_container.c
%
% Carlo Capelli <cc.carlo.cap@gmail.com> 2015

:- use_module(library(plgi)).

% note: producing the typelib required by PLGI can be done (once built and installed graph-gtk)
%
% $ g-ir-scanner --nsversion=1.0 --warn-all --pkg=graph-gtk --namespace=GraphGtk --include=Gtk-3.0 --library=graph-gtk --output GraphGtk.gir *.[ch]
% $ g-ir-compiler --output=GraphGtk.typelib GraphGtk.gir
%
:- plgi_use_namespace('GraphGtk').

gg_test :-
	gtk_window_new('GTK_WINDOW_TOPLEVEL', Window),
	gtk_window_set_default_size(Window, 800, 600),
	g_signal_connect(Window, destroy, gtk_main_quit/0, {null}, _),

	graph_gtk_view_new(GraphView),
	g_signal_connect(GraphView, 'node-selected', node_selected/3, {null}, _),
	g_signal_connect(GraphView, 'node-deselected', node_deselected/3, {null}, _),
	g_signal_connect(GraphView, 'nodes-connected', nodes_connected/6, {null}, _),
	g_signal_connect(GraphView, 'nodes-disconnected', nodes_disconnected/6, {null}, _),

	gtk_box_new('GTK_ORIENTATION_VERTICAL', 0, Vbox),
	gtk_box_set_homogeneous(Vbox, false),
	gtk_container_add(Window, Vbox),

	gtk_menu_bar_new(Menubar),
	gtk_menu_new(Graph_menu),
	gtk_menu_item_new_with_label('Graph', Graph),
	gtk_menu_item_set_submenu(Graph, Graph_menu),

	gtk_menu_item_new_with_label('Add Node', Add_node),
	g_signal_connect(Add_node, activate, menu_item_activated/2, GraphView, _),
	gtk_menu_shell_append(Graph_menu, Add_node),

	gtk_menu_item_new_with_label('Delete Node', Delete),
	g_signal_connect(Delete, activate, menu_item_activated/2, GraphView, _),
	gtk_menu_shell_append(Graph_menu, Delete),

	gtk_menu_item_new_with_label('Auto-arrange', Auto_arrange),
	g_signal_connect(Auto_arrange, activate, menu_item_activated/2, GraphView, _),
	gtk_menu_shell_append(Graph_menu, Auto_arrange),

	gtk_box_pack_start(Vbox, Menubar, false, false, 0),
	gtk_box_pack_start(Vbox, GraphView, true, true, 0),

	gtk_menu_shell_append(Menubar, Graph),

	gtk_widget_show_all(Window),
	gtk_main.

menu_item_activated(Menu_item, GraphView) :-
	gtk_menu_item_get_label(Menu_item, Label),
writeln(gtk_menu_item_get_label(Menu_item, Label)),

	(   Label == 'Add Node'
	->  % gtk_widget_get_ancestor(Menu_item, 'GTK_TYPE_WINDOW', W),
	    gtk_dialog_new(Dialog),
	    gtk_window_set_title(Dialog, 'Add Node'),
	    %gtk_window_set_transient_for(Dialog, W),
	    gtk_window_set_modal(Dialog, true),
	    gtk_dialog_add_button(Dialog, '_OK', 'GTK_RESPONSE_OK', _),
	    gtk_dialog_add_button(Dialog, '_CANCEL', 'GTK_RESPONSE_CANCEL', _),
	    gtk_window_set_default_size(Dialog, 350, 80),

	    gtk_dialog_get_content_area(Dialog, Vbox),
	    gtk_label_new('Name', Name_label),
	    gtk_entry_new(Name_entry),
	    gtk_box_pack_start(Vbox, Name_label, false, false, 0),
	    gtk_box_pack_start(Vbox, Name_entry, false, false, 0),

	    gtk_box_new('GTK_ORIENTATION_HORIZONTAL', 0, Columns),
	    gtk_box_set_homogeneous(Columns, false),
	    gtk_box_pack_start(Vbox, Columns, true, true, 0),

	    gtk_box_new('GTK_ORIENTATION_VERTICAL', 0, Left_column),
	    gtk_box_set_homogeneous(Left_column, false),
	    gtk_box_pack_start(Columns, Left_column, true, true, 0),
	    gtk_label_new('Inputs', InputsL),
	    gtk_box_pack_start(Left_column, InputsL, false, false, 0),

	    gtk_box_new('GTK_ORIENTATION_VERTICAL', 0, Right_column),
	    gtk_box_set_homogeneous(Right_column, false),
	    gtk_box_pack_start(Columns, Right_column, true, true, 0),
	    gtk_label_new('Outputs', OutputsL),
	    gtk_box_pack_start(Right_column, OutputsL, false, false, 0),

	    gtk_box_new('GTK_ORIENTATION_VERTICAL', 0, Inputs),
	    gtk_box_set_homogeneous(Inputs, false),
	    gtk_box_pack_start(Left_column, Inputs, true, true, 0),

	    gtk_box_new('GTK_ORIENTATION_VERTICAL', 0, Outputs),
	    gtk_box_set_homogeneous(Outputs, false),
	    gtk_box_pack_start(Right_column, Outputs, true, true, 0),

	    gtk_button_new_with_label('Add', Add_input),
	    g_signal_connect(Add_input, 'clicked', button_clicked/2, Inputs, _),
	    gtk_box_pack_start(Left_column, Add_input, false, false, 0),

	    gtk_button_new_with_label('Add', Add_output),
	    g_signal_connect(Add_output, 'clicked', button_clicked/2, Outputs, _),
	    gtk_box_pack_start(Right_column, Add_output, false, false, 0),

	    gtk_widget_show_all(Vbox),

	    gtk_dialog_run(Dialog, Result),
	    (   plgi_enum_value('GTK_RESPONSE_OK', Result)
	    ->  graph_gtk_node_new(Node),
/***/		gtk_entry_get_text(Name_entry, NodeName),
/***/		graph_gtk_node_set_name(Node, NodeName),

/***/		gtk_container_get_children(Inputs, ListInputs),
		forall(member(Entry, ListInputs), (
		       % I don't know how to code
		       % anyway, all Entries are GtkEntry here...
		       % if(GTK_IS_ENTRY(entry))
/***/		       gtk_entry_get_text(Entry, PadText),
		       graph_gtk_node_add_pad(Node, PadText, false)
		       )),

/***/		gtk_container_get_children(Outputs, ListOutputs),
		forall(member(Entry, ListOutputs), (
/***/		       gtk_entry_get_text(Entry, PadText),
		       graph_gtk_node_add_pad(Node, PadText, true)
		       )),

		graph_gtk_view_add_node(GraphView, Node)

	    ;   writeln(cancel)
	    ),
	    gtk_window_close(Dialog)
	;   Label == 'Delete Node'
	->  graph_gtk_view_remove_selected_nodes(GraphView)
	;   Label == 'Auto-arrange'
	->  graph_gtk_view_arrange(GraphView)
	;   print_message(error, unknown_menu_label(Label))
	).

node_selected(View, Node, Data) :-
	writeln(node_selected(View, Node, Data)).
node_deselected(View, Node, Data) :-
	writeln(node_deselected(View, Node, Data)).
nodes_connected(View, From, Output, To, Input, Data) :-
	writeln(nodes_connected(View, From, Output, To, Input, Data)).
nodes_disconnected(View, From, Output, To, Input, Data) :-
	writeln(nodes_disconnected(View, From, Output, To, Input, Data)).

