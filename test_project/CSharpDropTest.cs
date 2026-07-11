using Godot;

// C# counterpart of fall_test.gd: verifies the .NET scripting layer works
// unchanged on top of the Box3D physics extension (M0 baseline check).
public partial class CSharpDropTest : Node3D
{
	private int _frames;
	private RigidBody3D _body;

	public override void _Ready()
	{
		GD.Print("Active physics engine setting: ",
			ProjectSettings.GetSetting("physics/3d/physics_engine"));

		var ground = new StaticBody3D();
		var groundShape = new CollisionShape3D();
		groundShape.Shape = new BoxShape3D { Size = new Vector3(10, 1, 10) };
		ground.AddChild(groundShape);
		ground.Position = new Vector3(0, -0.5f, 0);
		AddChild(ground);

		_body = new RigidBody3D();
		var bodyShape = new CollisionShape3D();
		bodyShape.Shape = new SphereShape3D { Radius = 0.5f };
		_body.AddChild(bodyShape);
		_body.Position = new Vector3(0, 5, 0);
		AddChild(_body);

		GD.Print("Initial body Y: ", _body.Position.Y);
	}

	public override void _PhysicsProcess(double delta)
	{
		_frames++;
		if (_frames == 120)
		{
			float y = _body.GlobalPosition.Y;
			GD.Print("Body Y after 120 physics frames: ", y);
			GD.Print("Body linear velocity: ", _body.LinearVelocity);
			if (y < 4.0f && y > -5.0f)
				GD.Print("RESULT: PASS - C# body fell under gravity and did not tunnel through ground");
			else
				GD.Print("RESULT: FAIL - C# body did not fall as expected");
			GetTree().Quit();
		}
	}
}
