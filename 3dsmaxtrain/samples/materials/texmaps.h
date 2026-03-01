//-----------------------------------------------------------------------------
//  Texmaps
//-----------------------------------------------------------------------------

#define MAX_TEXTURE_CHANNELS	32

class TexmapSlot {
	public:
		float amount;
		Control *amtCtrl;  // ref to controller
		Texmap	*map;       // ref to map
		BOOL	mapOn;
		TSTR	name;

		TexmapSlot();
		RGBA Eval(ShadeContext& sc) { 
			return amount * map->EvalColor(sc); 
			}
		float EvalMono(ShadeContext& sc) { 
			return amount * map->EvalMono(sc); 
			}
		float LerpEvalMono(ShadeContext& sc, float v) {
			if( amount<0.0f ){
				float b = 1.0f + amount;
				if ( b < 0.0f ) b = 0.0f;
				return -amount * (1.0f - map->EvalMono(sc)) + b*v; 
			} else {
				float b = 1.0f - amount;
				if ( b < 0.0f ) b = 0.0f;
				return amount * map->EvalMono(sc) + b*v; 
			}
		}

		float LerpMono( float origVal, float texVal ) // for reShading, no eval
		{
			if( amount<0.0f )
			{
				float b = 1.0f + amount;
				if ( b < 0.0f ) b = 0.0f;
				return -amount * texVal + b * (1.0f - origVal); 
			} else {
				float b = 1.0f - amount;
				if ( b < 0.0f ) b = 0.0f;
				return amount * texVal + b * origVal;
			}
		}

		Point3 EvalNormalPerturb(ShadeContext &sc) {
			return amount * map->EvalNormalPerturb(sc); 
			}
		BOOL IsActive() { 
			return (map&&mapOn&&(amtCtrl||amount!=0.0f)); 
		}
		void Update(TimeValue t, Interval &ivalid);				
		float GetAmount(TimeValue t);
		BOOL HasMap() { return map != 0 ? TRUE : FALSE; }

};


class Texmaps: public TexmapContainer {
	public:  
		MtlBase *client;
		TexmapSlot txmap[MAX_TEXTURE_CHANNELS];
		BOOL loadingOld;
		ReferenceTarget* mLastNotifyTarget;

		Texmaps();
		Texmaps(MtlBase *mb);
		void SetClientPtr(MtlBase *mb) { client = mb; }
		TexmapSlot& operator[](int i) { return txmap[i]; }
		ReferenceTarget* GetLastNotifyTarget();

		Class_ID ClassID();
		virtual void GetClassName(MSTR& s, bool localized) const override { s = _M("Texmaps"); }

		void DeleteThis();
		RefTargetHandle Clone(RemapDir &remap);	
		RefResult NotifyRefChanged( const Interval& changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate );

		void RescaleWorldUnits(float f);

		BOOL AssignController(Animatable *control,int subAnim) {
			ReplaceReference(SubNumToRefNum(subAnim),(ReferenceTarget *)control);
			return TRUE;
			}

		int NumSubs();
	    Animatable* SubAnim(int i);
		TSTR SubAnimName(int i, bool localized) override;
		int SubNumToRefNum(int subNum) {return subNum; }
		BOOL InvisibleProperty() { return TRUE; }  // maps are made visible in scripter by pb_maps paramblock in the material so don't expose them as a subanim
		
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		void SetName( long i, TSTR& nm ){txmap[i].name = nm; } 
		TSTR& GetName( long i ){return txmap[i].name; } 
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		BOOL HasSomeMaps(){
			for(int i=0; i<STD2_NMAX_TEXMAPS; ++i){
				if( txmap[i].map )
					return TRUE;
			}
			return FALSE;
		}

	};


