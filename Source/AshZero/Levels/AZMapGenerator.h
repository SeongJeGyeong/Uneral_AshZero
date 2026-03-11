// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZMapGenerator.generated.h"

class AAZRoomBase;

UENUM(BlueprintType)
enum class EProceduralStates : uint8
{
	NONE,
	SPAWNROOMS,
	SEPARATEROOMS,
	HIGHLIGHTMAINROOMS,
	DISTANTIATEROOMS,
	DRAWDELTRIANGLES,
	DRAWMINSPANTREE,
	DRAWHALLWAYS
};

USTRUCT(Atomic)
struct FTriEdge
{
	GENERATED_USTRUCT_BODY()

	FTriEdge() {}
	FTriEdge(FVector2D P1, FVector2D P2)
	{
		Point1 = P1;
		Point2 = P2;
	}

	UPROPERTY()
	FVector2D Point1;
	UPROPERTY()
	FVector2D Point2;

	bool bIsBad = false;
};

USTRUCT(Atomic)
struct FTriangle
{
	GENERATED_USTRUCT_BODY()

	FTriangle() {}
	FTriangle(FVector2D P1, FVector2D P2, FVector2D P3)
	{ 
		Point1 = P1;
		Point2 = P2;
		Point3 = P3;
	}

	UPROPERTY()
	FVector2D Point1;
	UPROPERTY()
	FVector2D Point2;
	UPROPERTY()
	FVector2D Point3;

	bool bIsBad = false;

	bool CircumCircleContains(const FVector2D& Point) const
	{
		const double ab = Point1.X * Point1.X + Point1.Y * Point1.Y;
		const double cd = Point2.X * Point2.X + Point2.Y * Point2.Y;
		const double ef = Point3.X * Point3.X + Point3.Y * Point3.Y;

		const double ax = Point1.X;
		const double ay = Point1.Y;
		const double bx = Point2.X;
		const double by = Point2.Y;
		const double cx = Point3.X;
		const double cy = Point3.Y;

		const double circum_x = (ab * (cy - by) + cd * (ay - cy) + ef * (by - ay)) / (ax * (cy - by) + bx * (ay - cy) + cx * (by - ay));
		const double circum_y = (ab * (cx - bx) + cd * (ax - cx) + ef * (bx - ax)) / (ay * (cx - bx) + by * (ax - cx) + cy * (bx - ax));

		const FVector2D circum(circum_x / 2, circum_y / 2);
		const double circum_radius = FVector2D::DistSquared(Point1, circum);
		const double dist = FVector2D::DistSquared(Point, circum);
		return dist <= circum_radius;
	}
};

USTRUCT(Atomic)
struct FMinSpanTree
{
	GENERATED_USTRUCT_BODY()

public:
	TArray<std::pair<FVector2D, FVector2D>> GetNaturalCostPairs()
	{
		Size = CostPairs.Num();
		FillRootMap();
		FVector2D A, B;
		double cost = 0.f;
		TArray<std::pair<FVector2D, FVector2D>> Result;
		for (std::pair<double, std::pair<FVector2D, FVector2D>> CostPair : CostPairs)
		{
			A = CostPair.second.first;
			B = CostPair.second.second;
			cost = CostPair.first;
			// check if roots are creating a cycle
			if (GetRoot(A) != GetRoot(B))
			{
				_minCost += cost;
				Result.Add({ A, B });
				AddPair(A, B);
			}
			else
			{
				if (3 == (rand() % 9)) Result.Add({ A, B });
			}
		}
		return Result;
	}

	void FillRootMap()
	{
		for (std::pair<double, std::pair<FVector2D, FVector2D>> CostPair : CostPairs)
		{
			std::pair<FVector2D, FVector2D> Pair = CostPair.second;
			RootMap.Add(Pair.first, Pair.first);
			RootMap.Add(Pair.second, Pair.second);
		}
	}

	FVector2D GetRoot(FVector2D val)
	{
		while (RootMap[val] != val)
		{
			RootMap[val] = RootMap[RootMap[val]];
			val = RootMap[val];

		}
		return val;
	}

	void AddPair(FVector2D a, FVector2D b)
	{
		FVector2D aR = GetRoot(a);
		FVector2D bR = GetRoot(b);
		RootMap[aR] = RootMap[bR];
	}

public:
	TArray<std::pair<double, std::pair<FVector2D, FVector2D>>> CostPairs;
private:
	TMap<FVector2D, FVector2D> RootMap;
	double _minCost;
	int32 Size;
};

UCLASS()
class ASHZERO_API AAZMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZMapGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = MapGenerator)
	EProceduralStates CurrentState = EProceduralStates::NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = MapGenerator)
	int32 RoomRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = MapGenerator)
	int32 TotalRoomsToSpawn;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AAZRoomBase>> Rooms;

	UPROPERTY()
	TArray<TObjectPtr<AAZRoomBase>> SelectedRooms;

	UPROPERTY()
	TMap<FVector2D, TObjectPtr<AAZRoomBase>> RoomLocMap;

	UPROPERTY()
	TArray<FTriangle> DelaunayTriangles;

	TArray<std::pair<FVector2D, FVector2D>> MinPairs;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AAZRoomBase> SpawningRoom;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SpawnRoom();

	UFUNCTION(BlueprintCallable)
	void SperateOverlappingRooms();

	UFUNCTION(BlueprintCallable)
	void HighlightMainRooms();

	UFUNCTION(BlueprintCallable)
	void DistantiateRooms(float distance);

	UFUNCTION(BlueprintCallable)
	void DrawDelTriangles();

	UFUNCTION(BlueprintCallable)
	void DrawMinSpTree();

	UFUNCTION(BlueprintCallable)
	void DrawHallways();

private:
	FVector GetRandomPointInCircle(float Radius);
	TArray<FTriangle> Triangulate(TArray<FVector2D>& vertices);
	bool AlmostEqual(const FTriEdge& Edge1, const FTriEdge& Edge2);
	bool AlmostEqual(const FVector2D& V1, const FVector2D& V2);
	bool AlmostEqual(double x, double y, double rel_tol = 1e-12, double abs_tol = std::numeric_limits<double>::denorm_min());
	bool IsContainsVertex(const FTriangle& Triangle, const FVector2D& Point);
};
