/*--------------------------------------------//
World definition
This is where the simulation is controlled
//--------------------------------------------*/
#ifndef WORLD
#define WORLD
	/*--------------------------------------------//
	Includes
	//--------------------------------------------*/
		#include "world.h"

	/*--------------------------------------------//
	Constructors
	//--------------------------------------------*/
		/*--------------------------------------------//
		Default constructor
		//--------------------------------------------*/
			world::world(){
				//variable defaults
					objCnt = 0;
					objects = NULL;
					transCnt = 0;
					transparents = NULL;
					slightCnt = 0;
					slights = NULL;
					dlightCnt = 0;
					dlights = NULL;
					gravObjCnt = 0;
					gravObj = NULL;

					timestep = 1;
					timeout = 1;
					vertexrad = 1.0;
					gravObjMass = 100;
					gravConst = 100;
					frictionDist = 100;
					frictionConst = 100;
					deformConst = 0.1;

				ShaderInit();
			};

		/*--------------------------------------------//
		Overloaded constructor
		//--------------------------------------------*/
			world::world(int ts, int to, double vr, double gom, double gc, double fd, double fc, double dc):world(){
				timestep = ts;
				timeout = to;
				vertexrad = vr;
				gravObjMass = gom;
				gravConst = gc;
				frictionDist = fd;
				frictionConst = fc;
				deformConst = dc;
			};

	/*--------------------------------------------//
	Destructor
	//--------------------------------------------*/
		world::~world(){
			for (int i = 0; i < this->getObjectCount(); i++){
				free ((mesh*)(this->getObject(i)));
			}
		};

	/*--------------------------------------------//
	ShaderInit - Initialize shaders
	//--------------------------------------------*/
		void world::ShaderInit(void){
			SimpleShader = new Shader("SimpleShader","shaders/SimpleShader.vertex", "shaders/SimpleShader.fragment", NULL);
			ShadowShader = new Shader("ShadowShader","shaders/shadowShader.vertex", "shaders/shadowShader.fragment", NULL);
		}

	/*--------------------------------------------//
	Objects
	//--------------------------------------------*/
		/*--------------------------------------------//
		Regular physics objects that are non transparent
		//--------------------------------------------*/
			/*--------------------------------------------//
			Load object(s) from a file
			Returns the parent object created
			//--------------------------------------------*/
				bool world::loadObj(char* objPath, char* mtlPath, mesh** parent){
					//temp variables
					std::vector<mesh*>			temp_meshes (0);
					std::vector<texture*>   	temp_textures (0);			//textures of each material
					std::vector<char*>			temp_mat_names (0);			//names of materials index corresponds to texture index
					std::vector<vec4*> 			temp_vertices (0);			//vertices
					std::vector<vec2*> 			temp_uvs (0);				//texture coordinates
					std::vector<vec3*> 			temp_normals (0);			//normals
					mesh* 				   		curr_obj = NULL;		//current object
					texture* 			   		curr_texture = NULL;	//current texture

					//open mateial file
					FILE* mtlfile = fopen(mtlPath, "r");
					if (mtlfile == NULL){
					    printf("Failed to open file %s!\n", mtlPath);
					    return false;
					}
					//process material file
					while (1){
						//find how long current line is
						int length = 1;
						for (char c = getc(mtlfile); c != EOF; c = getc(mtlfile)){
							if (c == '\n')
								break;
							length++;
						}
						//go back what we just read
						fseek(mtlfile, -length-1, SEEK_CUR);
						//create a buffer of appropriate size
						char mtlHeader[length];
						//Read the first world of the line and ensure we are not at end of file
					    if (fscanf(mtlfile, "%s", mtlHeader) == EOF)
					        break;

					    if (strcmp(mtlHeader, "newmtl") == 0){
					    	char* name = (char*)malloc(sizeof(char)*(length-8));
					    	fscanf(mtlfile, "%s\n", name);
						    temp_mat_names.push_back(name);
						    continue;
					    }
					    if (strcmp(mtlHeader, "map_Kd") == 0){
					    	char* file = (char*)malloc(sizeof(char)*(length-8));
					    	fscanf(mtlfile, "%s\n", file);
					    	texture* material = new texture(file, 1);
					    	temp_textures.push_back(material);
					    	continue;
					    }
					    //read until next line
						for (char c = getc(mtlfile); c != EOF; c = getc(mtlfile)){
							if (c == '\n')
								break;
						}
					}

					//open object file
					FILE* objfile = fopen(objPath, "r");
					if (objfile == NULL){
					    printf("Failed to open file %s!\n", objPath);
					    return false;
					}

					//process object file
					while (1){
						//find how long current line is
						int length = 1;
						for (char c = getc(objfile); c != EOF; c = getc(objfile)){
							if (c == '\n')
								break;
							length++;
						}
						//go back what we just read
						fseek(objfile, -length-1, SEEK_CUR);
						//create a buffer of appropriate size
						char lineHeader[length];
						 //Read the first world of the line and check if at end of file
					    if (fscanf(objfile, "%s\n", lineHeader) == EOF)
					        break;

					    //check what kind of line we are
						if (strcmp(lineHeader, "usemtl") == 0){
							//use material
							char mat_name[length-8];
							fscanf(objfile, "%s", mat_name);
							for(unsigned int i = 0; i < temp_mat_names.size(); i++){
								if (strcmp(mat_name, temp_mat_names[i]) == 0){
									//found the material we are looking for
									curr_texture = temp_textures[i];
									break;
								}
							}
							continue;
						}
					    if (strcmp(lineHeader, "v") == 0){
					    	//position vector
					    	vec4* vertex = new vec4(0,0,0,1);
						    fscanf(objfile, "%lf %lf %lf\n", &vertex->x, &vertex->y, &vertex->z);
						    temp_vertices.push_back(vertex);
						    continue;
					    }
					    if (strcmp(lineHeader, "vt") == 0){
					    	//texture vector
						    vec2* uv = new vec2(0,0);
						    fscanf(objfile, "%lf %lf\n", &uv->x, &uv->y );
						    temp_uvs.push_back(uv);
						    continue;
						}
						if (strcmp(lineHeader, "vn") == 0){
							//normal vector
						    vec3* normal = new vec3(0,0,0);
						    fscanf(objfile, "%lf %lf %lf\n", &normal->x, &normal->y, &normal->z);
						    temp_normals.push_back(normal);
						    continue;
						}
						if (strcmp(lineHeader, "f") == 0){
							//triangle
						    unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
						    int matches = fscanf(objfile, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
						    if (matches != 9){
						        printf("File can't be read by our simple parser. Try exporting with other options\n");
						        fclose(objfile);
						        return false;
						    }
						    vertex* a = new vertex(temp_vertices[vertexIndex[0] - 1], temp_normals[normalIndex[0] - 1], new vec4(1,1,1,1));
							vertex* b = new vertex(temp_vertices[vertexIndex[1] - 1], temp_normals[normalIndex[0] - 1], new vec4(1,1,1,1));
							vertex* c = new vertex(temp_vertices[vertexIndex[2] - 1], temp_normals[normalIndex[0] - 1], new vec4(1,1,1,1));
							triangle* t = new triangle(a,b,c, temp_uvs[uvIndex[0] - 1], double(0), temp_uvs[uvIndex[1] - 1], double(0), temp_uvs[uvIndex[2] - 1], double(0));
							t->setMat(curr_texture, 0);
							curr_obj->addTri(t);
							continue;
						}
						if (strcmp(lineHeader, "o") == 0){
							//object
							if (curr_obj == NULL){
								//first object
								curr_obj = new mesh();
								*parent = curr_obj;
							}else{
								curr_obj = new mesh();
							}
						    temp_meshes.push_back(curr_obj);
						}
						//read until next line
						for (char c = getc(objfile); c != EOF; c = getc(objfile)){
							if (c == '\n')
								break;
						}
					}
					for(unsigned int i = 0; i < temp_meshes.size(); i++){
						this->addMesh(temp_meshes[i]);
					}
					fclose(objfile);
					return true;
				}

			/*--------------------------------------------//
			Adds a object to the world space
			takes a mesh as a parameter
			//--------------------------------------------*/
				void world::addMesh(mesh* &obj){
					mesh** newobjs = (mesh**) realloc(objects, sizeof(mesh*)*(objCnt+1));

					if (newobjs!=NULL) {
						objects = newobjs;

						objects[objCnt] = obj;
						objCnt++;
					}else{
						puts ("Error (re)allocating memory on world");
						exit (1);
					}
				};

			/*--------------------------------------------//
			Removes the specified mesh from the world
			does nothing if it is not found
			//--------------------------------------------*/
				void world::remMesh(mesh* &obj){
					for (int i = 0; i < objCnt; i++){
						//find object to remove
						if (obj == objects[i]){
							//move last object to here
							objects[i] = objects[objCnt-1];
							//trim last object
							mesh** newobjs = (mesh**) realloc(objects, sizeof(mesh*)*(objCnt-1));
							//check if memory was allocated
							if (newobjs!=NULL) {
								remGravObj(obj);
								objects = newobjs;
								objCnt--;
							}else{
								puts ("Error (re)allocating memory");
								exit (1);
							}
							return;
						}
					}
				};

			/*--------------------------------------------//
			Getters
			//--------------------------------------------*/
				int world::getObjectCount(){
					return objCnt;
				};

				mesh* world::getObject(int i){
					return objects[i];
				};

		/*--------------------------------------------//
		Gravity Objects
		//--------------------------------------------*/
			/*--------------------------------------------//
			Adds a object to the gravity object list
			takes a mesh as a parameter
			//--------------------------------------------*/
				void world::addGravObj(mesh* &obj){
					mesh** newobjs = (mesh**) realloc(gravObj, sizeof(mesh*)*(gravObjCnt+1));

					if (newobjs!=NULL) {
						gravObj = newobjs;

						gravObj[gravObjCnt] = obj;
						gravObjCnt++;
					}else{
						puts ("Error (re)allocating memory");
						exit (1);
					}
				};

			/*--------------------------------------------//
			Removes the specified mesh as a gravity object
			does nothing if it is not found
			//--------------------------------------------*/
				void world::remGravObj(mesh* &obj){
					for (int i = 0; i < gravObjCnt; i++){
						//find object to remove
						if (obj == gravObj[i]){
							//move last object to here
							gravObj[i] = gravObj[gravObjCnt-1];
							//trim last object
							mesh** newobjs = (mesh**) realloc(gravObj, sizeof(mesh*)*(gravObjCnt-1));
							//check if memory was allocated
							if (newobjs!=NULL) {
								gravObj = newobjs;
								gravObjCnt--;
							}else{
								puts ("Error (re)allocating memory");
								exit (1);
							}
							return;
						}
					}
				};

		/*--------------------------------------------//
		Transparent objects
		//--------------------------------------------*/
			/*--------------------------------------------//
			Adds a object to the world space
			takes a mesh as a parameter
			//--------------------------------------------*/
				void world::addTrans(mesh* &obj){
					mesh** newtrans = (mesh**) realloc(transparents, sizeof(mesh*)*(transCnt+1));

					if (newtrans!=NULL) {
						transparents = newtrans;

						transparents[transCnt] = obj;
						transCnt++;
					}else{
						puts ("Error (re)allocating memory on world");
						exit (1);
					}
				};

			/*--------------------------------------------//
			Removes the specified mesh from the world
			does nothing if it is not found
			//--------------------------------------------*/
				void world::remTrans(mesh* &obj){
					for (int i = 0; i < transCnt; i++){
						//find object to remove
						if (obj == transparents[i]){
							//move last object to here
							transparents[i] = transparents[transCnt-1];
							//trim last object
							mesh** newtrans = (mesh**) realloc(transparents, sizeof(mesh*)*(transCnt-1));
							//check if memory was allocated
							if (newtrans!=NULL) {
								remGravObj(obj);
								transparents = newtrans;
								transCnt--;
							}else{
								puts ("Error (re)allocating memory");
								exit (1);
							}
							return;
						}
					}
				};

			/*--------------------------------------------//
			Getters
			//--------------------------------------------*/
				int world::getTransCount(){
					return transCnt;
				};

				mesh* world::getTrans(int i){
					return transparents[i];
				};

		/*--------------------------------------------//
		Lights
		//--------------------------------------------*/
			/*--------------------------------------------//
			Directional Lights
			//--------------------------------------------*/
				/*--------------------------------------------//
				Adds a light to the world space
				takes a light as a parameter
				//--------------------------------------------*/
					void world::addDLight(direclight* &l){
						direclight** newdlights = (direclight**) realloc(dlights, sizeof(direclight*)*(dlightCnt+1));

						if (newdlights!=NULL) {
							dlights = newdlights;

							dlights[dlightCnt] = l;
							dlightCnt++;
						}else{
							puts ("Error (re)allocating memory on world");
							exit (1);
						}
					};

				/*--------------------------------------------//
				Removes the specified light from the world
				does nothing if it is not found
				//--------------------------------------------*/
					void world::remDLight(direclight* &l){
						for (int i = 0; i < dlightCnt; i++){
							//find light to remove
							if (l == dlights[i]){
								//move last light to here
								dlights[i] = dlights[dlightCnt-1];
								//trim last light
								direclight** newdlights = (direclight**) realloc(dlights, sizeof(direclight*)*(dlightCnt-1));
								//check if memory was allocated
								if (newdlights!=NULL) {
									dlights = newdlights;
									dlightCnt--;
								}else{
									puts ("Error (re)allocating memory");
									exit (1);
								}
								return;
							}
						}
					};

				/*--------------------------------------------//
				Getters
				//--------------------------------------------*/
					int world::getDLightCount(){
						return dlightCnt;
					}

					direclight* world::getDLight(int i){
						return dlights[i];
					}

			/*--------------------------------------------//
			Spot Lights
			//--------------------------------------------*/
				/*--------------------------------------------//
				Adds a light to the world space
				takes a light as a parameter
				//--------------------------------------------*/
					void world::addSLight(spotlight* &l){
						spotlight** newslights = (spotlight**) realloc(slights, sizeof(spotlight*)*(slightCnt+1));

						if (newslights!=NULL) {
							slights = newslights;

							slights[slightCnt] = l;
							slightCnt++;
						}else{
							puts ("Error (re)allocating memory on world");
							exit (1);
						}
					};

				/*--------------------------------------------//
				Removes the specified light from the world
				does nothing if it is not found
				//--------------------------------------------*/
					void world::remSLight(spotlight* &l){
						for (int i = 0; i < slightCnt; i++){
							//find light to remove
							if (l == slights[i]){
								//move last light to here
								slights[i] = slights[slightCnt-1];
								//trim last light
								spotlight** newslights = (spotlight**) realloc(slights, sizeof(spotlight*)*(slightCnt-1));
								//check if memory was allocated
								if (newslights!=NULL) {
									slights = newslights;
									slightCnt--;
								}else{
									puts ("Error (re)allocating memory");
									exit (1);
								}
								return;
							}
						}
					};

				/*--------------------------------------------//
				Getters
				//--------------------------------------------*/
					int world::getSLightCount(){
						return slightCnt;
					}

					spotlight* world::getSLight(int i){
						return slights[i];
					}

	/*--------------------------------------------//
	Misc
	//--------------------------------------------*/
		/*--------------------------------------------//
		Get Physics Timestep for world
		//--------------------------------------------*/
			int world::getTimeStep(){
				return timestep;
			};

		/*--------------------------------------------//
		Checks if an object is awake or not
		does this by compairing to our timeout value
		//--------------------------------------------*/
			bool world::isAwake(mesh* &obj){
				return obj->getTimer() > timeout;
			};

		/*--------------------------------------------//
		Implicit test function
		checks if two implicit function defined spheres
		intersect
		//--------------------------------------------*/
			bool world::implicitTest(vec3 pos1, vec3 pos2, double radi1, double radi2, vec3 vel1, vec3 vel2){
				//calculate x portion
				(pos2.x > pos1.x) ? /*Decide which direction to go*/
					(pos1.x + radi1 + vel1.x > pos2.x - radi2 - vel2.x) ? pos1.x = pos2.x : pos1.x = pos1.x + radi1 + radi2 + vel1.x - vel2.x:
					(pos1.x - radi1 - vel1.x > pos2.x + radi2 + vel2.x) ? pos1.x = pos2.x : pos1.x = pos1.x - radi1 - radi2 + vel1.x - vel2.x;

				//calculate y portion
				(pos2.y > pos1.y) ? /*Decide which direction to go*/
					(pos1.y + radi1 + vel1.y > pos2.y - radi2 - vel2.y) ? pos1.y = pos2.y : pos1.y = pos1.y + radi1 + radi2 + vel1.y - vel2.y:
					(pos1.y - radi1 - vel1.y > pos2.y + radi2 + vel2.y) ? pos1.y = pos2.y : pos1.y = pos1.y - radi1 - radi2 + vel1.y - vel2.y;

				//calculate z portion
				(pos2.z > pos1.z) ? /*Decide which direction to go*/
					(pos1.z + radi1 + vel1.z > pos2.z - radi2 - vel2.z) ? pos1.z = pos2.z : pos1.z = pos1.z + radi1 + radi2 + vel1.z - vel2.z:
					(pos1.z - radi1 - vel1.z > pos2.z + radi2 + vel2.z) ? pos1.z = pos2.z : pos1.z = pos1.z - radi1 - radi2 + vel1.z - vel2.z;

				return (pos2.x*pos2.x + pos2.y*pos2.y + pos2.z*pos2.z - radi1*radi1 - radi2*radi2 <= 0);
			};

	/*--------------------------------------------//
	Physics
	//--------------------------------------------*/
		/*--------------------------------------------//
		Update function
		proceeds all objects forward by one timestep
		//--------------------------------------------*/
			void world::update(){
				for (int i = 0; i < objCnt; ++i){
					applyGravity(objects[i]);
					testBpCollision(objects[i], i);
					objects[i]->updateAcc();
					objects[i]->updateVel();
					objects[i]->updatePos();
					if (objects[i]->getMass() > gravObjMass){
						addGravObj(objects[i]);
					}else{
						remGravObj(objects[i]);
					}
				}
				return;
			};

		/*--------------------------------------------//
		Gravity function
		calculates gravity for inputted object
		//--------------------------------------------*/
			void world::applyGravity(mesh* &obj){
				//only simulate gravity for objects listed as gravity objects
				for (int i = 0; i < gravObjCnt; i++){
					mesh* obj2 = gravObj[i];
					vec3 pos1 = obj->getPosition();
					vec3 pos2 = obj2->getPosition();
					double mass1 = obj->getMass();
					double mass2 = obj2->getMass();

					vec3 dir = pos1 - pos2;
					double distance = (dir).length();
					dir.normalize();

					obj->applyForce( dir * gravConst * mass1 * mass2 / pow(distance, 2) );
				}
			};

		/*--------------------------------------------//
		Friction function
		calculates friction for inputted object
		//--------------------------------------------*/
			void world::applyFriction(mesh* &obj){
				for (int i = 0; i < obj->getNearbyCnt(); i++){
					//setup variables
					mesh* obj2 = obj->getNearby(i);
					vec3 pos1 = obj->getPosition();
					vec3 pos2 = obj2->getPosition();
					vec3 vel1 = obj->getVelocity();
					vec3 vel2 = obj2->getVelocity();
					int mass1 = obj->getMass();
					int mass2 = obj2->getMass();
					double radi1 = obj->getRadius();
					double radi2 = obj2->getRadius();

					//find net velocity
					vec3 velNet = vel1 - vel2;
					//find force being applied, p = mv
					vec3 force = velNet / timestep * (mass1 + mass2);
					//define a projected line from obj1 along velocity
					line projection = line(pos1, velNet);
					vec3 intercept = vec3(0,0,0);
					double u = 0.0;
					//find how close obj1 and obj2 get
					double distance = projection.distance(pos2, intercept, u);
					//now apply an estimate for the friction force
					obj->applyForce(force * frictionConst * (radi1 + radi2 - distance));
					obj2->applyForce(force * frictionConst * (radi1 + radi2 - distance));
				}
			};

		/*--------------------------------------------//
		Broadphase Collision detection
		narrows down the possible objects that a given
		object could be colliding with.
		This is done using the 'meshes' 'radius'
		attribute and treating it as a implicit bounding 
		sphere and explicit bounding cube.
		This allows for implicit object simplification.
		//--------------------------------------------*/
			void world::testBpCollision(mesh* &obj, int i){
				//store objects that could be colliding
				mesh** filter = (mesh**) malloc(sizeof(mesh*)*objCnt-1-i);
				int filterCnt = 0;
				//loop over all objects starting after 'i'th item.  the 'i'th item should be the current item
				for (i++; i < objCnt-i; i++){
					mesh* obj2 = objects[i];
					//no self collision or collision between two 'sleeping' objects (objects not in motion)
					if(obj2 != obj && (isAwake(obj) || isAwake(obj2))){
						vec3 vel1 = obj->getVelocity() + obj->getForce()/obj->getMass();
						vec3 vel2 = obj2->getVelocity() + obj2->getForce()/obj2->getMass();

						//implicit function testing
						if(implicitTest(obj->getPosition(), obj2->getPosition(), obj->getRadius()+frictionDist, obj2->getRadius()+frictionDist, vel1, vel2)){
							//bounding sphere and bounding cube collide.  collision is possible
							filter[filterCnt] = obj2;
							filterCnt++;
							//add each other to the nearby lists
							obj->addNearby(obj2);
							obj2->addNearby(obj);
						}else{
							//remove each other as nearby objects
							obj->remNearby(obj2);
							obj2->remNearby(obj);
						}
					}else{
						//remove each other as nearby objects
						obj->remNearby(obj2);
						obj2->remNearby(obj);
					}
				}
				//-- From this point forward we start to affect more than one object, be careful with multithreading --//
				//apply friction to this object before any collision resolution occurs
				applyFriction(obj);

				//proceed to narrow phase collision detection with filter objects
				if(filterCnt > 0){
					testNpCollision(obj, filter, filterCnt);
				}
				free (filter);
				return;
			};

		/*--------------------------------------------//
		Narrowphase Collision detection
		Sorts the objects by an estimated collision time
		Then resolves collisions vertex by vertex
		//--------------------------------------------*/
			void world::testNpCollision(mesh* &obj, mesh** &filter, int &filterCnt){
				//We first need to sort by order of collision
				HeapSort heap;
				double times[filterCnt];
				for (int i = 0; i < filterCnt; i++){
					//setup variables
					mesh* obj2 = filter[i];
					vec3 vel1 = obj->getVelocity() + obj->getForce()/obj->getMass();
					vec3 vel2 = obj2->getVelocity() + obj2->getForce()/obj2->getMass();
					vec3 pos1 = obj->getPosition();
					vec3 pos2 = obj2->getPosition();

					//estimate time
					vec3 velNet = vel1 - vel2;
					vec3 intercept = vec3(0,0,0);
					line projection = line(pos1, velNet);
					double u = 0.0;
					projection.distance(pos2, intercept, u);
					times[i] = u;
				}
				//finally use our time estimates to sort our filter
				heap = HeapSort((void**)filter, (double**)&times, filterCnt);

				//loop over all objects that could still collide
				for (int i = 0; i < filterCnt; i++){
					//setup variables
					mesh* obj2 = filter[i];
					vec3 vel1 = obj->getVelocity() + obj->getForce()/obj->getMass();
					vec3 vel2 = obj2->getVelocity() + obj2->getForce()/obj2->getMass();
					vec3 pos1 = obj->getPosition();
					vec3 pos2 = obj2->getPosition();
					double radi1 = obj->getRadius();
					double radi2 = obj2->getRadius();

					//decide how much force goes into deformation and movement
					//p = mv, m1v1 + m2v2 = (m1+m2)v3
					vec3 resultVel = (vel1 * obj->getMass() + vel2 * obj2->getMass()) / (obj->getMass() + obj2->getMass());
					vec3 moveFrc1 = (resultVel - vel1) / timestep * obj->getMass();//units might match but probably not 100% formula accurate
					vec3 moveFrc2 = (resultVel - vel2) / timestep * obj2->getMass();//should give a good approximation though
					vec3 deformFrc1 = obj->getMaxDisplacement(moveFrc1);
					vec3 deformFrc2 = obj2->getMaxDisplacement(moveFrc2);
					moveFrc1 = moveFrc1 - deformFrc1;
					moveFrc2 = moveFrc2 - deformFrc2;

					//apply move forces
					obj->applyForce(moveFrc1);
					obj2->applyForce(moveFrc2);

					//we are going to adjust vertex positions now (apply deformation)
					//tad inefficent... loops over the vertices on both objects, hopefully narrows down quicker by implicit tests
					//loop over all vertices on object 1
					int vert1cnt;
					vertex** verts1;
					obj->getVertices(&verts1, &vert1cnt);
					for (int j = 0; j < vert1cnt; j++){
						vertex* vert1 = verts1[j];
						vec3 vert1pos = vec3(vert1->pos->x, vert1->pos->y, vert1->pos->z) + pos1;
						//implicit function test on vert1 to obj2
						if(implicitTest(vert1pos, pos2, vertexrad, radi2, vel1, vel2)){
							int vert2cnt;
							vertex** verts2;
							obj2->getVertices(&verts2, &vert2cnt);
							//loop over all vertices on object 2
							for (int k = 0; k < vert2cnt; k++){
								vertex* vert2 = verts2[k];
								vec3 vert2pos = vec3(vert2->pos->x, vert2->pos->y, vert2->pos->z) + pos2;
								//implicit function on vert2 to obj1
								if(implicitTest(pos1, vert2pos, radi1, vertexrad, vel1, vel2)){
									//both vert1 and vert2 might collide with something on the other object
									//now we check if vert1 and vert2 collide with each other
									//define a line from vert1 extending along the net velocity
									vec3 velNet = vel1 - vel2;
									line projection = line(vec3(*vert1->pos), velNet);
									vec3 intercept = vec3(0,0,0);
									double u = 0.0;
									//find distance, time, and intercept point
									double distance = projection.distance(vec3(*vert2->pos), intercept, u);

									if(distance > vertexrad){
										//vert1 and vert2 come within vertexrad distance at intercept point
										double time = (u - floor(u)) * timestep;//a value between 0->1 * timestep gives the amount of time into this timestep
										*vert1 -= deformFrc1 * deformConst / (distance * time * vert1cnt);//units might match but probably not 100% formula accurate
										*vert2 -= deformFrc2 * deformConst / (distance * time * vert2cnt);//should give a good approximation though
									}
								}
							}
						}
					}

					/*
					//create triangle filters
					triangle** tris1 = (triangle**)malloc(sizeof(triangle*)*obj->getTriangleCount());//overestimate malloc
					int tris1Cnt = 0;

					triangle** tris2 = (triangle**)malloc(sizeof(triangle*)*obj2->getTriangleCount());//overestimate malloc
					int tris2Cnt = 0;

					//loop over all of object1's triangles
					for (int j = 0; j < obj->getTriangleCount(); j++){
						//implicit function testing
						if(implicitTest(obj->getTriangle(j)->getPosition(), pos2, 1, radi2, vel1, vel2)){
							//passed test, add to triangle filter
							tris1[tris1Cnt] = obj->getTriangle(j);
							tris1Cnt++;
						}
					}

					//loop over all of object2's triangles
					for (int j = 0; j < obj2->getTriangleCount(); j++){
						//implicit function testing
						if(implicitTest(pos1, obj2->getTriangle(j)->getPosition(), radi1, 1, vel1, vel2)){
							//passed test, add to triangle filter
							tris2[tris2Cnt] = obj2->getTriangle(j);
							tris2Cnt++;
						}
					}

					//check if collision is still possible
					if(tris1Cnt + tris2Cnt == 0){
						free (tris1);
						free (tris2);
						continue;
					}

					//now test each triangle collision and assign weight for magnitudes of impact
					for (int j = 0; j < tris1Cnt; j++){
						triangle* tri1 = tris1[j];
						for (int k = 0; k < tris2Cnt; k++){
							triangle* tri2 = tris2[k];
							//check if avg point of tri2 intersects with tri1
							if(tri1->intersects(tri2->getPosition())){
								vec3 tri1pos = pos1 + tri1->getPosition();
								vec3 tri2pos = pos2 + tri2->getPosition();
								//find the dot product of the normals (how much 'deflection' occurs)
								double attack = tri1->getNormal().dot(tri2->getNormal());
								//find the proximity to the primary collision
								vec3 colVec = pos1 - pos2;
								vec3 triColVec = tri1pos - tri2pos;
								double triColDiff = (colVec - triColVec).length();
								double proximity1 = radi1 / triColDiff;
								double proximity2 = radi2 / triColDiff;

								//apply to each vertex of both triangles
								for (int l = 0; l < 3; l++){
									tris1[j*tris1Cnt + k]->getVertex(l)->x -= deformFrc1.x * proximity1 * attack;
									tris1[j*tris1Cnt + k]->getVertex(l)->y -= deformFrc1.y * proximity1 * attack;
									tris1[j*tris1Cnt + k]->getVertex(l)->z -= deformFrc1.z * proximity1 * attack;

									tris2[j*tris1Cnt + k]->getVertex(l)->x -= deformFrc2.x * proximity2 * attack;
									tris2[j*tris1Cnt + k]->getVertex(l)->y -= deformFrc2.y * proximity2 * attack;
									tris2[j*tris1Cnt + k]->getVertex(l)->z -= deformFrc2.z * proximity2 * attack;
								}
							}else{
								//no collision
							}
						}
					}
					//done with this check... clean up
					free (tris1);
					free (tris2);
					//*/
				}
			};

	/*--------------------------------------------//
	Drawing functions
	Uses a deferred rendering approach
	lighting is broken up into passes to simplify
	overall process
	//--------------------------------------------*/
		/*-------------------------------------------//
		renderCube() renders a 1x1 3D cube in NDC.
		//-------------------------------------------*/
			void world::renderCube()
			{
			    // initialize (if necessary)
			    if (cubeVAO == 0)
			    {
			        float vertices[] = {
			            // back face
			            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			            // front face
			            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			            // left face
			            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			            // right face
			             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			            // bottom face
			            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			            // top face
			            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			        };
			        glGenVertexArrays(1, &cubeVAO);
			        glGenBuffers(1, &cubeVBO);
			        // fill buffer
			        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			        // link vertex attributes
			        glBindVertexArray(cubeVAO);
			        glEnableVertexAttribArray(0);
			        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			        glEnableVertexAttribArray(1);
			        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			        glEnableVertexAttribArray(2);
			        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			        glBindBuffer(GL_ARRAY_BUFFER, 0);
			        glBindVertexArray(0);
			    }
			    // render Cube
			    glBindVertexArray(cubeVAO);
			    glDrawArrays(GL_TRIANGLES, 0, 36);
			    glBindVertexArray(0);
			}

		/*-------------------------------------------//
		renderQuad() renders a 1x1 XY quad in NDC
		//-------------------------------------------*/
			void world::renderQuad()
			{
			    if (quadVAO == 0)
			    {
			        float quadVertices[] = {
			            // positions        // texture Coords
			            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			        };
			        // setup plane VAO
			        glGenVertexArrays(1, &quadVAO);
			        glGenBuffers(1, &quadVBO);
			        glBindVertexArray(quadVAO);
			        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			        glEnableVertexAttribArray(0);
			        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			        glEnableVertexAttribArray(1);
			        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			    }
			    glBindVertexArray(quadVAO);
			    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			    glBindVertexArray(0);
			}

		/*-------------------------------------------//
		Sorting objects by distance to viewer using 
		insertion sort
		//-------------------------------------------*/
			void world::objectSort(glm::vec4 camera){
				int j, di, dj;
				mesh* key;
			
				for(int i = 1; i < objCnt; i++){
					key = objects[i];
					di = (vec3(camera.x, camera.y, camera.z) - key->getPosition()).length();
					j = i -1;
					dj = (vec3(camera.x, camera.y, camera.z) - objects[j]->getPosition()).length();

					while(j >= 0 && di < dj){
						objects[j+1] = objects[j];
						j = j - 1;
					
					}
					objects[j + 1] = key;
				}
			}
		
		/*--------------------------------------------//
		Overall Draw function
		//--------------------------------------------*/
			void world::draw(glm::mat4 projection, glm::mat4 view, glm::vec4 camera, GLint currWindowSize[2]){
				// Clear the screen
				glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//Position and orient camera.
				Mat4 LightProjectionMatrix, LightViewMatrix, CameraProjectionMatrix, CameraViewMatrix;
				std::vector<Mat4> shadowTransforms;
				Vec3 position = Vec3(0.0f, 0.0f, 0.0f );
				Quat quat = Quat(Vec3(viewerAltitude, viewerAzimuth, 0.0));
				Mat4 looking = Mat4(quat);
				Vec4 camera = looking * Vec4(position.x + 0.0f, position.y + 0.0f, position.z + 1.0f*viewerDistance, 1.0f);
				Vec4 up = looking * Vec4(0,1,0,1);

				//Calculate & save matrices
				CameraProjectionMatrix.Perspective(FRUSTUM_FIELD_OF_VIEW, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, (float)FRUSTUM_NEAR_PLANE, (float)FRUSTUM_FAR_PLANE);
				CameraViewMatrix.LookAt(camera, position, up);
				LightProjectionMatrix.Perspective(FRUSTUM_FIELD_OF_VIEW, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, (float)FRUSTUM_NEAR_PLANE, (float)FRUSTUM_FAR_PLANE);
				LightViewMatrix.LookAt(lightPos, position, Vec3(0,1,0));

				DepthShader->use();
				DepthShader->setMat4("CameraProjectionMatrix", LightProjectionMatrix);
				DepthShader->setMat4("CameraViewMatrix", LightViewMatrix);
				DepthShader->setMat4("LightProjectionMatrix", LightProjectionMatrix);
				DepthShader->setMat4("LightViewMatrix", LightViewMatrix);
				DepthShader->setVec3("LightPosition", lightPos);
				DepthShader->setVec3("FarPlane", FRUSTUM_FAR_PLANE);

				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
				RenderScene(LightViewMatrix, DepthShader);
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, currWindowSize[0], currWindowSize[1]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//render from camera
				ShadowMapping->use();
				ShadowMapping->setMat4("CameraProjectionMatrix", CameraProjectionMatrix);
				ShadowMapping->setMat4("CameraViewMatrix", CameraViewMatrix);
				ShadowMapping->setVec4("ViewPos", camera);
				ShadowMapping->setVec3("SpotLight.position", lightPos);
				ShadowMapping->setFloat("SpotLight.fov", FRUSTUM_FIELD_OF_VIEW);
				ShadowMapping->setFloat("SpotLight.constant", 0.1f);
				ShadowMapping->setFloat("SpotLight.linear", 0.1f);
				ShadowMapping->setFloat("SpotLight.exponential", 0.1f);
				ShadowMapping->setVec3("SpotLight.direc.direction", (-lightPos).GetNormalized());
				ShadowMapping->setVec3("SpotLight.direc.base.color", Vec3(white));
				ShadowMapping->setVec3("DirecLight.base.color", Vec3(white));
				ShadowMapping->setVec3("DirecLight.direction", (-lightPos).GetNormalized());
				ShadowMapping->setMat4("LightProjectionMatrix", LightProjectionMatrix);
				ShadowMapping->setMat4("LightViewMatrix", LightViewMatrix);
				ShadowMapping->setInt("DepthMap", 1);
				ShadowMapping->setInt("LightType", 0);
				ShadowMapping->setInt("texture1set", 0);
				ShadowMapping->setInt("texture2set", 0);
				ShadowMapping->setInt("ALIAS", 10);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

				RenderScene(CameraViewMatrix, ShadowMapping);

				glUseProgram(0);
			};
#endif
