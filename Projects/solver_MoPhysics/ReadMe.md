

             __  __       _____  _               _             _____       _                   __           
            |  \/  |     |  __ \| |             (_)           / ____|     | |                 / _|          
            | \  / | ___ | |__) | |__  _   _ ___ _  ___ ___  | (___   ___ | |_   _____ _ __  | |_ ___  _ __ 
            | |\/| |/ _ \|  ___/| '_ \| | | / __| |/ __/ __|  \___ \ / _ \| \ \ / / _ \ '__| |  _/ _ \| '__|
            | |  | | (_) | |    | | | | |_| \__ \ | (__\__ \  ____) | (_) | |\ V /  __/ |    | || (_) | |   
            |_|  |_|\___/|_|    |_| |_|\__, |___/_|\___|___/ |_____/ \___/|_| \_/ \___|_|    |_| \___/|_|   
                                        __/ |                                                               
                                       |___/                                                                
                    _            _           _      __  __       _   _             ____        _ _     _           
         /\        | |          | |         | |    |  \/  |     | | (_)           |  _ \      (_) |   | |          
        /  \  _   _| |_ ___   __| | ___  ___| | __ | \  / | ___ | |_ _  ___  _ __ | |_) |_   _ _| | __| | ___ _ __ 
       / /\ \| | | | __/ _ \ / _` |/ _ \/ __| |/ / | |\/| |/ _ \| __| |/ _ \| '_ \|  _ <| | | | | |/ _` |/ _ \ '__|
      / ____ \ |_| | || (_) | (_| |  __/\__ \   <  | |  | | (_) | |_| | (_) | | | | |_) | |_| | | | (_| |  __/ |   
     /_/    \_\__,_|\__\___/ \__,_|\___||___/_|\_\ |_|  |_|\___/ \__|_|\___/|_| |_|____/ \__,_|_|_|\__,_|\___|_|   
                                                                                                                   
                                                                                                                   

	Using Bullet physics engine. bullet-2.82-r2704

	For chassis and wheel constraint will use convex meshes, so please add some simple proxies instead of real car geometry!

	Tips
		Was failed to use x64 version of Newton library - error while init memory manager. Solve a problem by recompiling newton engine 
	souce code (core library) with flag __x86_64__

	Features:
	 * auto steering using a 3d path

	TODO:
	 !!! - fix a error when turn off solver - pure function call !
	 - dynamic chain properties type and solver for that. Important to implement motion inheritance and stiffness
	 - player properties to control a character, walking in the phys world


	LOG
	 29.07.2016
	  Updated newton to the recent version. Seems more stable collisions and simulation.
	   But recording has lots of bugs
	  More interesting properties like lock differential

	 26.07.2016
	  Added Curve Steering Weight to blend between manual steering value and curve steering

	 31.05.2016
	  Make a version for precise recording. Try to sync between phys time and anim time using the linear interpolation between
	  prev and current matrices

	 05.05.2015
		Make a fix to run a car from different null and collision object orientations.

	 29.03.2015
		Small refactoring of code, rename project to solver_MoPhysics, remove all unused files from the project
		We have a stable car simulation, problem was with polygon winding in a static collision geometry !

	 25.03.2015
		I have no idea how to make it work in stable and predictible manner. Seems that's the end for that project.

	 25.03.2015
		Made a physics solver and car physics properties object

	 17.03.2015
		New based architecture, physics common interface and static library for plugging newton engine

	 18.02.2015
		Start integrating newton physics one more time, based on last car simulation sample (with NPC path motion)

	 29.01.2014 change phys engine to Bullet, hope this time will be more stable simulation and collisions

	 25.01.2014 move to 2.36 engine version, seems that it can work back in MoBu 2014

	 21.01.2014 ported to MoBu 2010 32bit, first working demo

	 02.01.2014 first succeed compilation

	 28.12.2013 integrating "car in motion" into the mobu constraint
