// Fill out your copyright notice in the Description page of Project Settings.

#include "AZMapGenerator.h"
#include "Rooms/AZRoomBase.h"

// Sets default values
AAZMapGenerator::AAZMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAZMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAZMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case EProceduralStates::SPAWNROOMS:
		SpawnRoom();
		break;
	case EProceduralStates::SEPARATEROOMS:
		SperateOverlappingRooms();
		break;
	case EProceduralStates::HIGHLIGHTMAINROOMS:
		HighlightMainRooms();
		break;
	case EProceduralStates::DISTANTIATEROOMS:
		DistantiateRooms(1000.f);
		break;
	case EProceduralStates::DRAWDELTRIANGLES:
		DrawDelTriangles();
		break;
	case EProceduralStates::DRAWMINSPANTREE:
		DrawMinSpTree();
		break;
	case EProceduralStates::DRAWHALLWAYS:
		DrawHallways();
		break;
	default:
		break;
	}
}

void AAZMapGenerator::SpawnRoom()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning.............."));
	if (TotalRoomsToSpawn <= 0)
	{
		CurrentState = EProceduralStates::NONE;
		UE_LOG(LogTemp, Warning, TEXT("TotalRoomsToSpawn is zero"));
		return;
	}
	if (SpawningRoom)
	{
		for (int i = 0; i < TotalRoomsToSpawn; i++)
		{
			//spawn
			FActorSpawnParameters tParams;
			tParams.Owner = this;

			FRotator rot;
			FVector loc = GetActorLocation() + GetRandomPointInCircle(500);
			loc.Z = 226.f;
			rot = FRotator::ZeroRotator;

			AAZRoomBase* Room = GetWorld()->SpawnActor<AAZRoomBase>(SpawningRoom, loc, rot, tParams);

			int scaleX = FMath::RandRange(4, RoomRange);
			int scaleY = FMath::RandRange(4, RoomRange);

			// random scale
			FVector scale(scaleX, scaleY, FMath::RandRange(5, 7));

			Room->SetActorScale3D(scale);
			Room->Scale = scaleX + scaleY;
			Rooms.Add(Room);
		}
	}
	else // impoertant Error Logging
	{
		UE_LOG(LogTemp, Error, TEXT("No Actor found while spawning."));
	}

	// set timer for room
	//GetWorldTimerManager().SetTimer(m_TimerGenerateDT, this,
		//&AProceduralMapsCharacter::OnTimerEnd, m_TimeForMoveRooms, false);

	// change state
	CurrentState = EProceduralStates::SEPARATEROOMS;
}

void AAZMapGenerator::SperateOverlappingRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("Separating.............."));

	// do until all are separated
	bool bFlag = true;

	for (auto Room : Rooms)
	{
		if (!Room->IsSeparateOverlappingRooms())
		{
			bFlag = false;
		}
	}

	if (bFlag) // if all rooms are done separating
		CurrentState = EProceduralStates::HIGHLIGHTMAINROOMS;
}

void AAZMapGenerator::HighlightMainRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("Highlighting.............."));

	for (AAZRoomBase* Room : Rooms)
	{
		if (Room->Scale > 14 && 1 == rand() % 4)
		{
			Room->HighlightRoom();
			Room->bIsMain = true;
			SelectedRooms.Add(Room);
		}
		else
		{
			if (rand() % 5 > 0)
			{
				Room->Destroy();
			}
			else
			{
				Room->HighlightRoom();
				Room->bIsMain = true;
				SelectedRooms.Add(Room);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Remaining Rooms: %d"), SelectedRooms.Num());
	// change state
	CurrentState = EProceduralStates::DISTANTIATEROOMS;
}

void AAZMapGenerator::DistantiateRooms(float Distance)
{
	UE_LOG(LogTemp, Warning, TEXT("Distancing.............."));

	FVector selfLoc, thirdLoc;
	bool flag = true;
	//for each rooms if distance lees than given
	for (AAZRoomBase* CurrentRoom : SelectedRooms)
	{
		selfLoc = CurrentRoom->GetActorLocation();
		// check with rm
		for (AAZRoomBase* OtherRoom : SelectedRooms)
		{
			if (CurrentRoom != OtherRoom) // make sure they are not same
			{
				thirdLoc = OtherRoom->GetActorLocation();

				if (FVector::Distance(selfLoc, thirdLoc) < Distance)
				{
					flag = false;
					// move rooms
					FVector dir = selfLoc - thirdLoc;
					dir.Normalize();
					auto offset = dir * 2;
					CurrentRoom->AddActorLocalOffset(offset);
					OtherRoom->AddActorLocalOffset(-offset);
				}
			}
		}
	}

	if (flag) CurrentState = EProceduralStates::DRAWDELTRIANGLES;
}

void AAZMapGenerator::DrawDelTriangles()
{
	UE_LOG(LogTemp, Warning, TEXT("Draw Triangles.............."));

	TArray<FVector2D> points;

	for (auto Room : SelectedRooms)
	{
		FVector2D tmp;
		//dt::Vector2<double> tmp;
		tmp.X = Room->GetActorLocation().X;
		tmp.Y = Room->GetActorLocation().Y;
		points.Add(tmp);
	}

	DelaunayTriangles = Triangulate(points);
	UE_LOG(LogTemp, Warning, TEXT("Total Triangles: %d"), DelaunayTriangles.Num());

	// Draw triangles
	float z = 600.f;
	for (auto t : DelaunayTriangles) // for each triangle
	{
		// draw line for each Edge
		DrawDebugLine(GetWorld(), FVector(t.Point1, z), FVector(t.Point2, z), FColor::Black, false, 2.f, 0, 50);
		DrawDebugLine(GetWorld(), FVector(t.Point1, z), FVector(t.Point3, z), FColor::Black, false, 2.f, 0, 50);
		DrawDebugLine(GetWorld(), FVector(t.Point2, z), FVector(t.Point3, z), FColor::Black, false, 2.f, 0, 50);
		//DrawCircle(GetWorld(), FVector(a, z), FVector(a, z), FVector(a, z), FColor::Red, 40, 2, true);
	}

	DrawMinSpTree();
}

void AAZMapGenerator::DrawMinSpTree()
{
	UE_LOG(LogTemp, Warning, TEXT("Draw Min Sp Tree.............."));
	float z = 600.f;
	// ********************* MST **************************
	// create minimum spanning tree
	FMinSpanTree Mst;
	for (FTriangle triangle : DelaunayTriangles) // for each triangle
	{
		// get all three loc and enter Three as apir
		Mst.CostPairs.Add({ FVector2D::Distance(triangle.Point1, triangle.Point2), {triangle.Point1, triangle.Point2} });
		Mst.CostPairs.Add({ FVector2D::Distance(triangle.Point1, triangle.Point3), {triangle.Point1, triangle.Point3} });
		Mst.CostPairs.Add({ FVector2D::Distance(triangle.Point2, triangle.Point3), {triangle.Point2, triangle.Point3} });
	}

	UE_LOG(LogTemp, Warning, TEXT("Total pairs in MST: %d"), Mst.CostPairs.Num());

	MinPairs = Mst.GetNaturalCostPairs();
	UE_LOG(LogTemp, Warning, TEXT("Extra ballancing MST pairs : %d"), MinPairs.Num());

	for (std::pair<FVector2D, FVector2D> Pair : MinPairs)
	{
		FVector2D A = Pair.first;
		FVector2D B = Pair.second;
		DrawDebugLine(GetWorld(), FVector(A, z + 300), FVector(B, z + 300), FColor::Green, false, 4.f, 0, 50);
	}

	CurrentState = EProceduralStates::DRAWHALLWAYS;
}

void AAZMapGenerator::DrawHallways()
{
	UE_LOG(LogTemp, Warning, TEXT("Draw Hallways.............."));

	for (std::pair<FVector2D, FVector2D> Pair : MinPairs)
	{
		FVector2D A = Pair.first;
		FVector2D B = Pair.second;

		float xDiff = B.X - A.X;
		float yDiff = A.Y - B.Y;
		FVector HorizontalEnd(A.X + xDiff, A.Y, 300);
		FVector VerticalEnd(B.X, B.Y + yDiff, 300);

		DrawDebugLine(GetWorld(), FVector(A, 300), HorizontalEnd, FColor::Blue, true, -1.f, 0, 200);
		DrawDebugLine(GetWorld(), FVector(B, 300), VerticalEnd, FColor::Blue, true, -1.f, 0, 200);
	}

	CurrentState = EProceduralStates::NONE;
}

FVector AAZMapGenerator::GetRandomPointInCircle(float Radius)
{
	FVector Point;
	float r = Radius * sqrt(FMath::RandRange(0.0f, 1.0f));
	float theta = (FMath::RandRange(0.0f, 1.0f)) * 2 * PI;

	Point.X = r * cos(theta);
	Point.Y = r * sin(theta);

	return Point;
}

TArray<FTriangle> AAZMapGenerator::Triangulate(TArray<FVector2D>& vertices)
{
	// Store the vertices locally
	TArray<FVector2D> _vertices = vertices;

	// Determinate the super triangle
	double minX = vertices[0].X;
	double minY = vertices[0].Y;
	double maxX = minX;
	double maxY = minY;

	for (uint64_t i = 0; i < vertices.Num(); ++i)
	{
		if (vertices[i].X < minX) minX = vertices[i].X;
		if (vertices[i].Y < minY) minY = vertices[i].Y;
		if (vertices[i].X > maxX) maxX = vertices[i].X;
		if (vertices[i].Y > maxY) maxY = vertices[i].Y;
	}

	const double dx = maxX - minX;
	const double dy = maxY - minY;
	const double deltaMax = std::max(dx, dy);
	const double midx = (minX + maxX) / 2;
	const double midy = (minY + maxY) / 2;

	const FVector2D p1(midx - 20 * deltaMax, midy - deltaMax);
	const FVector2D p2(midx, midy + 20 * deltaMax);
	const FVector2D p3(midx + 20 * deltaMax, midy - deltaMax);

	TArray<FTriangle> Triangles;

	Triangles.Add(FTriangle(p1, p2, p3));

	for (auto p = vertices.begin(); p != vertices.end(); ++p)
	{
		TArray<FTriEdge> polygon;

		for (auto& Triangle : Triangles)
		{
			if (Triangle.CircumCircleContains(*p))
			{
				Triangle.bIsBad = true;
				polygon.Add(FTriEdge(Triangle.Point1, Triangle.Point2));
				polygon.Add(FTriEdge(Triangle.Point2, Triangle.Point3));
				polygon.Add(FTriEdge(Triangle.Point3, Triangle.Point1));
			}
		}

		Triangles.RemoveAll([](const FTriangle& triangle) { return triangle.bIsBad; });

		for (auto Edge1It = polygon.CreateIterator(); Edge1It; ++Edge1It)
		{
			for (auto Edge2It = Edge1It + 1; Edge2It; ++Edge2It)
			{
				if (AlmostEqual(*Edge1It, *Edge2It))
				{
					Edge1It->bIsBad = true;
					Edge2It->bIsBad = true;
				}
			}
		}

		polygon.RemoveAll([](const FTriEdge& edge) { return edge.bIsBad; });

		for (const auto e : polygon) Triangles.Add(FTriangle(e.Point1, e.Point2, *p));
	}


	Triangles.RemoveAll([this, p1, p2, p3](const FTriangle& triangle)
		{
			return IsContainsVertex(triangle, p1) || IsContainsVertex(triangle, p2) || IsContainsVertex(triangle, p3);
		});

	/*for (const auto t : Triangles)
	{
		_edges.push_back(Edge<T>{*t.a, * t.b});
		_edges.push_back(Edge<T>{*t.b, * t.c});
		_edges.push_back(Edge<T>{*t.c, * t.a});
	}*/

	return Triangles;
}

bool AAZMapGenerator::AlmostEqual(const FTriEdge& Edge1, const FTriEdge& Edge2)
{
	return (AlmostEqual(Edge1.Point1, Edge2.Point1) && AlmostEqual(Edge1.Point2, Edge2.Point2)) ||
		   (AlmostEqual(Edge1.Point1, Edge2.Point2) && AlmostEqual(Edge1.Point2, Edge2.Point1));
}


bool AAZMapGenerator::AlmostEqual(const FVector2D& V1, const FVector2D& V2)
{
	return AlmostEqual(V1.X, V2.X) && AlmostEqual(V1.Y, V2.Y);
}

bool AAZMapGenerator::AlmostEqual(double x, double y, double rel_tol, double abs_tol)
{
	if (std::isnan(x) || std::isnan(y)) return false;
	if (x == y) return true; // 정확히 같은 경우 (Inf, -Inf 포함)
	if (std::isinf(x) || std::isinf(y)) return false;

	double diff = std::fabs(x - y);
	double maxab = std::max(std::fabs(x), std::fabs(y));

	return diff <= std::max(abs_tol, rel_tol * maxab);
}

bool AAZMapGenerator::IsContainsVertex(const FTriangle& Triangle, const FVector2D& Point)
{
	return AlmostEqual(Triangle.Point1, Point) || AlmostEqual(Triangle.Point2, Point) || AlmostEqual(Triangle.Point3, Point);
}