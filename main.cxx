#include "pandaFramework.h"
#include "windowFramework.h"
#include "nodePath.h"
#include "clockObject.h"
#include "asyncTask.h"
#include "genericAsyncTask.h"
#include "bulletWorld.h"
#include "bulletPlaneShape.h"
#include "bulletSphereShape.h"
#include "bulletBoxShape.h"
#include "bulletCylinderShape.h"
#include "bulletConvexHullShape.h"
#include "cardMaker.h"
#include "texture.h"
#include "texturePool.h"
#include "textureStage.h"
#include "material.h"

#include "pandaSystem.h"
#include "audioManager.h"

#include "ambientLight.h"
#include "pointLight.h"


// Global stuff
PandaFramework framework;
WindowFramework *window;
AsyncTaskManager *task_mgr = AsyncTaskManager::get_global_ptr();
PT(ClockObject) globalClock = ClockObject::get_global_clock();
NodePath camera;
NodePath table, rolling_pin, lunch_box, astronaut, sky;
PT(AudioManager) AM;
PT(AudioSound) mySound;


BulletWorld *physics_world;

ClockObject *co = ClockObject::get_global_clock();
TexturePool *texture_pool = TexturePool::get_global_ptr();

/**
 * ##############################################
 * ###             Models Parameter           ###
 * ##############################################
 **/

double speed = 2;

double astronaut_height = 1;
double astronaut_pos_x = 0;
double astronaut_pos_y = 0;
double astronaut_pos_z = 2.7;//the foot of the astronaut

double rolling_pin_length = 2;
double rolling_pin_pos_x = 3;
double rolling_pin_pos_y = 0;
double rolling_pin_pos_z = 2.5;

double table_height=1.5;

double lunch_box_length = 0.8;
double lunch_box_pos_x = -3;
double lunch_box_pos_y = 0;
double lunch_box_pos_z = 3.5;

// ##############################################


void locate();
void init_astronaut();
void init_rolling_pin();
void init_lunch_box();
void init_table();
void init_floor();
void init_ball();
void init_sky();
void KeyboardHandler(const Event *eventPtr, void *dataPtr);
void inputBinding();
void init_light();


/**
 * This function reset the models original position but not the velocity force and other phisics quantities.
 **/
void locate(){
	table.set_pos_hpr(0,0,0,0,0,0);
	lunch_box.set_pos_hpr(lunch_box_pos_x, lunch_box_pos_y , lunch_box_length + lunch_box_pos_z, 45, 0, 0);
	astronaut.set_pos_hpr(astronaut_pos_x, astronaut_pos_y , astronaut_height + astronaut_pos_z, 180, 0, 0);
	rolling_pin.set_pos_hpr(rolling_pin_pos_x, rolling_pin_pos_y , rolling_pin_length + rolling_pin_pos_z, 45, 90, 0);
}


AsyncTask::DoneStatus update_scene(GenericAsyncTask* task, void* data){
	ClockObject *co = ClockObject::get_global_clock();
	physics_world ->do_physics(co->get_dt(), 10, 1.0 / 180.0);
	
	return	AsyncTask::DS_cont;
}


void KeyboardHandler(const Event *eventPtr, void *dataPtr)
{
    if(eventPtr->get_name() == "arrow_up")
		camera.set_hpr(camera.get_hpr()[0], camera.get_hpr()[1]+0.30, camera.get_hpr()[2]);

    if(eventPtr->get_name() == "arrow_down")
		camera.set_hpr(camera.get_hpr()[0], camera.get_hpr()[1]-0.30, camera.get_hpr()[2]);

    if(eventPtr->get_name() == "arrow_left")
		camera.set_hpr(camera.get_hpr()[0]+0.30, camera.get_hpr()[1], camera.get_hpr()[2]);

	if(eventPtr->get_name() == "arrow_right")
		camera.set_hpr(camera.get_hpr()[0]-0.30, camera.get_hpr()[1], camera.get_hpr()[2]);

    if(eventPtr->get_name() == "w")
		camera.set_pos(camera.get_pos()[0], camera.get_pos()[1]+0.15, camera.get_pos()[2]);

    if(eventPtr->get_name() == "s")
		camera.set_pos(camera.get_pos()[0], camera.get_pos()[1]-0.15, camera.get_pos()[2]);

    if(eventPtr->get_name() == "a")
		camera.set_pos(camera.get_pos()[0]-0.15, camera.get_pos()[1], camera.get_pos()[2]);

	if(eventPtr->get_name() == "d")
		camera.set_pos(camera.get_pos()[0]+0.15, camera.get_pos()[1], camera.get_pos()[2]);

	if(eventPtr->get_name() == "space"){
		mySound->play();
		init_ball();
    }
	if(eventPtr->get_name() == "c")
		camera.set_pos(camera.get_pos()[0], camera.get_pos()[1], camera.get_pos()[2]+0.15);

	if(eventPtr->get_name() == "v"){
		camera.set_pos(camera.get_pos()[0], camera.get_pos()[1], camera.get_pos()[2]-0.15);
    }
	if(eventPtr->get_name() == "escape")
		exit(EXIT_SUCCESS);

	if(eventPtr->get_name() == "r"){
		cout << "locate" << endl;
		locate();
	}
}



#if defined DEBUG
	void debugModeHandler(const Event *eventPtr, void *dataPtr){
	    BulletDebugNode *debugNode = (BulletDebugNode *)dataPtr;
	    static bool show_bounding_boxes_value = false;
	    static bool show_constraints_value = false;
	    static bool show_normals_value = false;
	    static bool show_wireframe_value = false;
	    if(eventPtr->get_name() == "f1"){
	        show_bounding_boxes_value = !show_bounding_boxes_value;
	        debugNode->show_bounding_boxes(show_bounding_boxes_value);
	    }
	    if(eventPtr->get_name() == "f2"){
	        show_constraints_value = !show_constraints_value;
	        debugNode->show_constraints(show_constraints_value);
	    }
	    if(eventPtr->get_name() == "f3"){
	        show_normals_value = !show_normals_value;
	        debugNode->show_normals(show_normals_value);
	    }
	    if(eventPtr->get_name() == "f4"){
	        show_wireframe_value = !show_wireframe_value;
	        debugNode->show_wireframe(show_wireframe_value);
	    }
	}
#endif

void inputBinding(){
	framework.define_key("escape", "exit", KeyboardHandler, 0);
	#if defined DEBUG
		framework.define_key("c", "camera up", KeyboardHandler, 0);
		framework.define_key("v", "camera down", KeyboardHandler, 0);
	#endif
	framework.define_key("r", "home locator", KeyboardHandler, 0);
	framework.define_key("arrow_up", "camera forward", KeyboardHandler, 0);
	framework.define_key("arrow_down", "camera backward", KeyboardHandler, 0);
	framework.define_key("arrow_left", "camera left", KeyboardHandler, 0);
	framework.define_key("arrow_right", "camera right", KeyboardHandler, 0);
	framework.define_key("w", "camera forward", KeyboardHandler, 0);
	framework.define_key("a", "camera backward", KeyboardHandler, 0);
	framework.define_key("s", "camera left", KeyboardHandler, 0);
	framework.define_key("d", "camera right", KeyboardHandler, 0);
	framework.define_key("space", "ball launch", KeyboardHandler, 0);
}

void init_light(){

	PT(PointLight) s_light;
	s_light = new PointLight("my s_light");
	s_light->set_color(LVecBase4f(1.0, 1.0, 1.0, 1));
	NodePath slnp = window->get_render().attach_new_node(s_light);
	slnp.set_pos(5, -5, 10);
	window->get_render().set_light(slnp);

	PT(PointLight) s_light_2;
	s_light_2 = new PointLight("my s_light");
	s_light_2->set_color(LVecBase4f(1.0, 1.0, 1.0, 1));
	NodePath slnp_2 = window->get_render().attach_new_node(s_light_2);
	slnp_2.set_pos(-5, -5, 10);
	window->get_render().set_light(slnp_2);

	PT(AmbientLight) a_light;
	a_light = new AmbientLight("my a_light");
	a_light->set_color(LVecBase4f(0.2, 0.2, 0.2, 1));
	NodePath alnp = window->get_render().attach_new_node(a_light);
	window->get_render().set_light(alnp);
}

int main(int argc, char *argv[]) {
	camera.get_child(0).set_pos(LVecBase3(0.0,-3.0,0.0));
	AM = AudioManager::create_AudioManager();
	mySound = AM->get_sound("/home/polpo/Desktop/ass_panda/aaa.ogg");
	AM->set_active(true);
	mySound->set_loop(true);

    framework.open_framework(argc, argv);
    framework.set_window_title("Panda3D - Assignment");
 
    window = framework.open_window();

    camera = window->get_camera_group();
	camera.set_pos(-0,-20, 4);
	camera.set_hpr(-0, -0, 0);
 
    physics_world = new BulletWorld () ;
    physics_world->set_gravity(0 , 0 , -9.8) ;
		
	
	window->enable_keyboard();	
	inputBinding();
	init_sky();
   	init_floor();
	init_astronaut();
	init_table();
	init_rolling_pin();
	init_lunch_box();

    PT(GenericAsyncTask) task;
    task = new GenericAsyncTask("Scene update" , &update_scene , (void *) NULL );
    task_mgr->add(task);

	init_light();

	#if defined DEBUG
	    PT(BulletDebugNode) bullet_dbg_node;
	    bullet_dbg_node = new BulletDebugNode("Debug");
	    bullet_dbg_node->show_bounding_boxes(false);
	    bullet_dbg_node->show_constraints(false);
	    bullet_dbg_node->show_normals(false);
	    bullet_dbg_node->show_wireframe(false);
	    NodePath np_dbg_node = window->get_render().attach_new_node(bullet_dbg_node);
	    np_dbg_node.show();
	    physics_world->set_debug_node(bullet_dbg_node);
	
		framework.define_key("f1", "callChangeDebugMode", &debugModeHandler, (void *) bullet_dbg_node);
	    framework.define_key("f2", "callChangeDebugMode", &debugModeHandler, (void *) bullet_dbg_node);
	    framework.define_key("f3", "callChangeDebugMode", &debugModeHandler, (void *) bullet_dbg_node);
	    framework.define_key("f4", "callChangeDebugMode", &debugModeHandler, (void *) bullet_dbg_node);
	#endif

    framework.main_loop();
    framework.close_framework();
 
    return 0;
}


/**
 * ##############################################
 * ###             Models Setup               ###
 * ##############################################
 **/

void init_astronaut(){
	
	BulletBoxShape *shape1 = new BulletBoxShape(LVecBase3f(astronaut_height*0.25,astronaut_height*0.20,astronaut_height*0.5));//legs
	BulletSphereShape *shape2 = new BulletSphereShape(astronaut_height*0.2);//head
	BulletBoxShape *shape3 = new BulletBoxShape(LVecBase3f(astronaut_height*0.5,astronaut_height*0.3,astronaut_height*0.4));//body
	BulletRigidBodyNode* astronaut_rigid_node = new BulletRigidBodyNode("Box");

	astronaut_rigid_node->set_mass(0.3);

	astronaut_rigid_node->add_shape(shape1, TransformState::make_pos(LPoint3f(0.0,0.10*astronaut_height,0.0)));//legs
	astronaut_rigid_node->add_shape(shape2, TransformState::make_pos(LPoint3f(0.0,0.0,astronaut_height*1.4)));//head
	astronaut_rigid_node->add_shape(shape3, TransformState::make_pos(LPoint3f(0.0,-0.05*astronaut_height,astronaut_height*0.8)));//body

	physics_world->attach_rigid_body(astronaut_rigid_node);
 
	astronaut = window->get_render().attach_new_node(astronaut_rigid_node);
	astronaut.set_pos_hpr(astronaut_pos_x, astronaut_pos_y , astronaut_height + astronaut_pos_z, 180, 0, 0);

	NodePath astronaut_model = window->load_model(framework.get_models(),"models/astronaut/astronaut");
	astronaut_model.reparent_to(window->get_render());
	astronaut_model.set_scale(0.5*astronaut_height);
	astronaut_model.set_pos(-0, 0, -astronaut_height*0.50);
	astronaut_model.set_hpr(0, 0, 0);

	astronaut_model.reparent_to(astronaut);
	astronaut_rigid_node->set_friction(0.6);
	
}

void init_rolling_pin(){
	
	BulletCylinderShape *shape1 = new BulletCylinderShape(0.1*rolling_pin_length,rolling_pin_length);
	BulletRigidBodyNode* rolling_pin_rigid_node = new BulletRigidBodyNode("Box");

	rolling_pin_rigid_node->set_mass(0.3);

	rolling_pin_rigid_node->add_shape(shape1, TransformState::make_pos(LPoint3f(0.0,0.0,0.0)));


	physics_world->attach_rigid_body(rolling_pin_rigid_node);
 
	rolling_pin = window->get_render().attach_new_node(rolling_pin_rigid_node);
	rolling_pin.set_pos_hpr(rolling_pin_pos_x, rolling_pin_pos_y , rolling_pin_length + rolling_pin_pos_z, 45, 90, 0);

	NodePath rolling_pin_model = window->load_model(framework.get_models(),"models/rolling/RollingPin");
	rolling_pin_model.reparent_to(window->get_render());
	rolling_pin_model.set_scale(rolling_pin_length);
	rolling_pin_model.set_pos(0, 0, 0);
	rolling_pin_model.set_hpr(0, 0, 90);

	rolling_pin_model.reparent_to(rolling_pin);
	rolling_pin_rigid_node->set_friction(0.6);
	rolling_pin_rigid_node->set_anisotropic_friction(0.8);
	
}

void init_sexy_girl(){
	
	BulletCylinderShape *shape1 = new BulletCylinderShape(0.1*rolling_pin_length,rolling_pin_length);
	BulletRigidBodyNode* rolling_pin_rigid_node = new BulletRigidBodyNode("Box");

	rolling_pin_rigid_node->set_mass(0.3);

	rolling_pin_rigid_node->add_shape(shape1, TransformState::make_pos(LPoint3f(0.0,0.0,0.0)));


	physics_world->attach_rigid_body(rolling_pin_rigid_node);
 
	rolling_pin = window->get_render().attach_new_node(rolling_pin_rigid_node);
	rolling_pin.set_pos_hpr(rolling_pin_pos_x, rolling_pin_pos_y , rolling_pin_length + rolling_pin_pos_z, 45, 90, 0);

	NodePath rolling_pin_model = window->load_model(framework.get_models(),"models/sexy_girl/sexy_girl");
	rolling_pin_model.reparent_to(window->get_render());
	rolling_pin_model.set_scale(rolling_pin_length);
	rolling_pin_model.set_pos(0, 0, 0);
	rolling_pin_model.set_hpr(0, 0, 90);

	rolling_pin_model.reparent_to(rolling_pin);
	rolling_pin_rigid_node->set_friction(0.6);
	rolling_pin_rigid_node->set_anisotropic_friction(0.8);
	
}

void init_lunch_box(){

	BulletBoxShape *shape1 = new BulletBoxShape(LVecBase3f(lunch_box_length*1.1,lunch_box_length*0.5,lunch_box_length*1.2));
	BulletRigidBodyNode* lunch_box_rigid_node = new BulletRigidBodyNode("Box");

	lunch_box_rigid_node->set_mass(0.3);

	lunch_box_rigid_node->add_shape(shape1, TransformState::make_pos(LPoint3f(0.0,0.0,0.0)));


	physics_world->attach_rigid_body(lunch_box_rigid_node);
 
	lunch_box = window->get_render().attach_new_node(lunch_box_rigid_node);
	lunch_box.set_pos_hpr(lunch_box_pos_x, lunch_box_pos_y , lunch_box_length + lunch_box_pos_z, 45, 0, 0);

	NodePath lunch_box_model = window->load_model(framework.get_models(),"models/lunchBox/LunchBox");
	lunch_box_model.reparent_to(window->get_render());
	lunch_box_model.set_scale(lunch_box_length*3);
	lunch_box_model.set_pos(0, 0, 0);
	lunch_box_model.set_hpr(0, 0, 90);

	lunch_box_model.reparent_to(lunch_box);
	lunch_box_rigid_node->set_friction(0.6);
	lunch_box_rigid_node->set_anisotropic_friction(0.8);
	
}

void init_table(){
	LVecBase3f normal(5 , 3 , 2);

	double h = table_height;
	double p1x =  3.65*h;
	double p1y =  0.00;

	double p2x =  3.35*h;
	double p2y =  0.90*h;

	double p3x =  2.45*h;
	double p3y =  1.75*h;

	double p4x =  1.30*h;
	double p4y =  2.30*h;
	
	double p5x =  0.00*h;
	double p5y =  2.40*h;
	

	BulletConvexHullShape *shape1 = new BulletConvexHullShape();

	double thickness = 0.1*h;
	shape1->add_point(LPoint3f(p1x, p1y, thickness));//
	shape1->add_point(LPoint3f(p2x, p2y, thickness));
	shape1->add_point(LPoint3f(p3x, p3y, thickness));
	shape1->add_point(LPoint3f(p4x, p4y, thickness));
	shape1->add_point(LPoint3f(p5x, p5y, thickness));//
	shape1->add_point(LPoint3f(-p4x, p4y, thickness));
	shape1->add_point(LPoint3f(-p3x, p3y, thickness));
	shape1->add_point(LPoint3f(-p2x, p2y, thickness));
	shape1->add_point(LPoint3f(-p1x, p1y, thickness));//
	shape1->add_point(LPoint3f(-p2x, -p2y, thickness));
	shape1->add_point(LPoint3f(-p3x, -p3y, thickness));
	shape1->add_point(LPoint3f(-p4x, -p4y, thickness));
	shape1->add_point(LPoint3f(-p5x, -p5y, thickness));//
	shape1->add_point(LPoint3f(p4x, -p4y, thickness));
	shape1->add_point(LPoint3f(p3x, -p3y, thickness));
	shape1->add_point(LPoint3f(p2x, -p2y, thickness));
	
	shape1->add_point(LPoint3f(p1x, p1y, 0));//
	shape1->add_point(LPoint3f(p2x, p2y, 0));
	shape1->add_point(LPoint3f(p3x, p3y, 0));
	shape1->add_point(LPoint3f(p4x, p4y, 0));
	shape1->add_point(LPoint3f(p5x, p5y, 0));//
	shape1->add_point(LPoint3f(-p4x, p4y, 0));
	shape1->add_point(LPoint3f(-p3x, p3y, 0));
	shape1->add_point(LPoint3f(-p2x, p2y, 0));
	shape1->add_point(LPoint3f(-p1x, p1y, 0));//
	shape1->add_point(LPoint3f(-p2x, -p2y, 0));
	shape1->add_point(LPoint3f(-p3x, -p3y, 0));
	shape1->add_point(LPoint3f(-p4x, -p4y, 0));
	shape1->add_point(LPoint3f(-p5x, -p5y, 0));//
	shape1->add_point(LPoint3f(p4x, -p4y, 0));
	shape1->add_point(LPoint3f(p3x, -p3y, 0));
	shape1->add_point(LPoint3f(p2x, -p2y, 0));

	BulletBoxShape *shape2 = new BulletBoxShape(LVecBase3f(0.2*h,0.2*h,h));
	BulletBoxShape *shape3 = new BulletBoxShape(LVecBase3f(0.2*h,0.2*h,h));
	BulletBoxShape *shape4 = new BulletBoxShape(LVecBase3f(0.2*h,0.2*h,h));
	BulletBoxShape *shape5 = new BulletBoxShape(LVecBase3f(0.2*h,0.2*h,h));
	BulletBoxShape *shape6 = new BulletBoxShape(LVecBase3f(2.6*h,1.6*h,0.2*h));
	BulletRigidBodyNode* table_rigid_node = new BulletRigidBodyNode("Box");

	table_rigid_node->set_mass(1.0);

	table_rigid_node->add_shape(shape1, TransformState::make_pos(LPoint3f(0.0,0.0,2*h)));
	table_rigid_node->add_shape(shape2, TransformState::make_pos(LPoint3f(-2.8*h,-0.9*h,h)));
	table_rigid_node->add_shape(shape3, TransformState::make_pos(LPoint3f(-2.8*h,0.9*h,h)));
	table_rigid_node->add_shape(shape4, TransformState::make_pos(LPoint3f(2.8*h,-0.9*h,h)));
	table_rigid_node->add_shape(shape5, TransformState::make_pos(LPoint3f(2.8*h,0.9*h,h)));
	table_rigid_node->add_shape(shape6, TransformState::make_pos(LPoint3f(0.0,0.0,1.9*h)));

	physics_world->attach_rigid_body(table_rigid_node);
 
	table = window->get_render().attach_new_node(table_rigid_node);
	table.set_pos_hpr(0, 0, 0, 0, 0, 0);


	NodePath table_model = window->load_model(framework.get_models(),"models/dining_table_2/DiningTable2");
	table_model.reparent_to(window->get_render());
	table_model.set_scale(0.77*h);
	table_model.set_pos(0, 0, 0);
	table.set_hpr(0, 0, 0);
    
	table_model.reparent_to(table);
	
}

void init_floor(){  
  LVecBase3f normal (0 , 0 , 1) ;
  double d = 1;

  BulletPlaneShape * floor_shape = new BulletPlaneShape (normal, d);
  BulletRigidBodyNode * floor_rigid_node = new BulletRigidBodyNode("Ground");
  floor_rigid_node -> add_shape (floor_shape,TransformState::make_pos(LPoint3f(0.0,0.0,-1.0)));
  NodePath np_ground = window -> get_render().attach_new_node(floor_rigid_node);
  np_ground.set_pos(0, 30, 0);
  physics_world -> attach_rigid_body(floor_rigid_node);
  

  CardMaker* cm = new CardMaker("ground");
  cm->set_frame(-60, 60, -60, 60);

  NodePath np_ground_tex = window->get_render().attach_new_node(cm->generate());
  np_ground_tex.reparent_to(np_ground);
  
  TextureStage* ts = new TextureStage("ts");
  ts->set_mode(TextureStage::M_modulate);

	Texture* tex;
	tex = texture_pool->load_texture("floor1.jpg");
  
	np_ground_tex.set_p(270);
	np_ground_tex.set_tex_scale(ts, 9, 9);
	np_ground.set_texture(ts, tex);
	floor_rigid_node->set_friction(0.4);

	PT(Material) floor_material = new Material();
	floor_material->set_diffuse(LVecBase4(1,1,1,1));
	np_ground.set_material(floor_material);
	
}

void init_ball(){
	double radius = 0.1;

	
	LVecBase3f ang_speed(10 , 0 , 0);
	BulletSphereShape* sphere_shape = new BulletSphereShape( radius ) ;
	BulletRigidBodyNode* sphere_rigid_node = new BulletRigidBodyNode("Sphere");

	sphere_rigid_node->set_mass(0.02);
	sphere_rigid_node->add_shape(sphere_shape);

	physics_world->attach_rigid_body(sphere_rigid_node);
		
	NodePath np_sphere = window->get_render().attach_new_node(sphere_rigid_node);
	np_sphere.set_pos(camera.get_pos()[0], camera.get_pos()[1], camera.get_pos()[2]);
	np_sphere.set_hpr(camera.get_hpr()[0], -camera.get_hpr()[1], camera.get_hpr()[2]);


	NodePath np_sphere_model = window->load_model(framework.get_models(), "models/baseball/baseball");
	np_sphere_model.reparent_to(np_sphere);
	np_sphere_model.set_scale(radius*7);
	
	LVector3 vector_direction = camera.get_relative_vector(window->get_render(), camera.get_pos());
  	LVector3 bullet_direction = speed*LVector3(vector_direction[0], -vector_direction[1], vector_direction[2]);
	cout << bullet_direction << endl;
	sphere_rigid_node->set_linear_velocity(bullet_direction);
	sphere_rigid_node->set_angular_velocity(ang_speed);
}

void init_sky(){
	sky = window->load_model(framework.get_models(),"models/sky/blue_sky_sphere");
	sky.reparent_to(window->get_render());
	sky.set_scale(1);
	sky.set_pos(0, 0, -1);
	sky.set_hpr(0, 0, 0);
}

// ##############################################
