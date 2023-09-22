#include "plugin.hpp"
#include "scale.hpp"
#include <random>
#include <string>
#include <thread>
#include <chrono>

struct Melodygen : Module
{
	enum ParamId
	{
		RANGE_PARAM,
		SCALE_PARAM,
		KEY_PARAM,
		DISJUNCT_PARAM,
		RESTPROB_PARAM,
		TRILLPROB_PARAM,
		REPEAT_PARAM,
		ROOTGRAVITY_PARANM,
		TRILLRATE_PARAM,
		PARAMS_LEN
	};
	enum InputId
	{
		GATE_INPUT,
		INPUTS_LEN
	};
	enum OutputId
	{
		GATE_OUTPUT,
		CV_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId
	{
		BLINK_LIGHT,
		LIGHTS_LEN
	};

	float genProb()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.0, 1.0);
		return dis(gen);
	}

	Melodygen()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(KEY_PARAM, 0.0, 11.0, 0.0, "Key");
		configParam(SCALE_PARAM, 0.0, ScaleUtils::NUM_SCALES - 1, 0, "Scale");

		configParam(DISJUNCT_PARAM, 0.f, 1.f, 0.5, "Disjunction");
		configParam(RANGE_PARAM, 1, 5, 3, "Octave Range"); // Snapping to integer values between 1 and 5

		configParam(RESTPROB_PARAM, 0.f, 1.f, 0.f, "Probability of Rest");
		configParam(TRILLPROB_PARAM, 0.f, 1.f, 0.f, "Probability of Trill");
		configParam(REPEAT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ROOTGRAVITY_PARANM, 0.f, 1.f, 0.f, "% Root Gravity (this will override note repeat %!)");
		configParam(TRILLRATE_PARAM, 0.f, 10.f, 0.f, "Rate of Trill");

		configInput(GATE_INPUT, "GATE IN");
		configOutput(GATE_OUTPUT, "GATE OUT");
		configOutput(CV_OUTPUT, "CV OUT");
	}

	float lastCV = 0.f;
	int lastRandIndex = 0;
	bool gateOn = false;
	bool skipNote = false;
	bool trilling = false;
	float nextTrillCV;
	float lastTrillCV;
	int sampleCounter = 0;
	int trillCount = 0;

	void process(const ProcessArgs &args) override
	{
		int trillRate = static_cast<int>((10.f - static_cast<float>(params[TRILLRATE_PARAM].getValue())) * 1000.f);
		float gateIn = static_cast<float>(inputs[GATE_INPUT].getVoltage());
		if (trilling)
		{
			if (sampleCounter >= trillRate)
			{
				trillCount++;
				sampleCounter = 0;
				if (trillCount % 2 == 0)
				{
					outputs[CV_OUTPUT].setVoltage(nextTrillCV);
				}
				else
				{
					outputs[CV_OUTPUT].setVoltage(lastTrillCV);
				}
			}
			sampleCounter++;
			if (trillCount == 8)
			{
				trilling = false;
				trillCount = 0;
				sampleCounter = 0;
			}
			else
			{
				return;
			}
		}

		if (skipNote && gateIn >= 10.f)
		{
			return;
		}
		else if (skipNote && gateIn < 10.f)
		{
			skipNote = false;
		}

		// Read the scale knob and key knobs and determine the selected scale
		int scaleIndex = static_cast<int>(params[SCALE_PARAM].getValue());
		int keyValue = static_cast<int>(params[KEY_PARAM].getValue());
		int *selectedScale = ScaleUtils::SCALE_ARRAY[scaleIndex];
		int scaleSize = ScaleUtils::SCALE_SIZE[scaleIndex];

		// Get prob params
		float disjunctValue = static_cast<float>(params[DISJUNCT_PARAM].getValue());
		float restProb = static_cast<float>(params[RESTPROB_PARAM].getValue());
		float rootProb = static_cast<float>(params[ROOTGRAVITY_PARANM].getValue());
		float trillProb = static_cast<float>(params[TRILLPROB_PARAM].getValue());
		float repeatProb = static_cast<float>(params[REPEAT_PARAM].getValue());

		// Get the range in octaves from the knob
		int range = static_cast<int>(params[RANGE_PARAM].getValue());

		// Total number of notes across all octaves
		int totalNotes = range * scaleSize;

		// Check for positive edge of gate
		if (gateIn >= 10.f && !gateOn)
		{
			// Enforce window
			if ((lastCV > 12.f) || (lastCV < 0.f))
			{
				lastCV = 0.f;
				lastRandIndex = 0;
				gateOn = false;
				skipNote = false;
				trilling = false;
				nextTrillCV = 0;
				lastTrillCV = 0;
				sampleCounter = 0;
				trillCount = 0;
				return;
			}
			// Determine if new note will happen
			float randRoll = genProb();

			if (randRoll < restProb)
			{
				outputs[GATE_OUTPUT].setVoltage(0.f);
				skipNote = true;
				return;
			}

			std::random_device rd;
			std::mt19937 gen(rd());

			std::uniform_int_distribution<> dis;
			int randomNoteIndex;

			std::vector<int> adjacentNotes;

			float repeatRoll = genProb();

			int notesBelowWeighted = static_cast<int>(static_cast<float>(lastRandIndex) * disjunctValue);
			int notesAboveWeighted = static_cast<int>(static_cast<float>(totalNotes - lastRandIndex - 1) * disjunctValue);

			int lowerBound = fmax(lastRandIndex - notesBelowWeighted, 0);
			int upperBound = fmin(lastRandIndex + notesAboveWeighted, totalNotes - 1);

			dis = std::uniform_int_distribution<>(lowerBound, upperBound);
			randomNoteIndex = dis(gen);

			// Check if the same note is selected as last time
			if ((randomNoteIndex == lastRandIndex) && ((repeatRoll >= repeatProb) || (repeatProb == 0)))
			{
				// Choose the note above or below it based on available range
				if (randomNoteIndex > 0 && randomNoteIndex < totalNotes - 1)
				{
					randomNoteIndex += (gen() % 2 == 0) ? 1 : -1;
				}
				else if (randomNoteIndex > 0)
				{
					randomNoteIndex -= 1;
				}
				else
				{
					randomNoteIndex += 1;
				}
			}

			int randomOctave = randomNoteIndex / scaleSize;
			int randomNote = selectedScale[randomNoteIndex % scaleSize] + (12 * randomOctave) + keyValue;

			// Calculate all root note positions
			std::vector<int> rootNoteIndexes;
			for (int i = 0; i <= range; ++i)
			{
				rootNoteIndexes.push_back(keyValue + i * 12); // Assuming keyValue is your root note and range is your max octave
			}

			// Find closest root note to randomNote
			int closestRoot = *std::min_element(rootNoteIndexes.begin(), rootNoteIndexes.end(),
												[&randomNote](const int &a, const int &b) -> bool
												{
													return std::abs(a - randomNote) < std::abs(b - randomNote);
												});

			float rootGravRoll = genProb();
			if (rootGravRoll < rootProb)
			{
				randomNote = closestRoot;
			}

			lastCV = (float)randomNote / 12.f; // Assuming 1V/Oct standard
			lastRandIndex = randomNoteIndex;
			outputs[GATE_OUTPUT].setVoltage(10.f);

			gateOn = true;

			// Determine if trill will happen
			float trillRoll = genProb();
			if (trillRoll < trillProb)
			{
				trilling = true;
			}
			int nextNote = selectedScale[(randomNoteIndex + 1) % scaleSize] + (12 * randomOctave) + keyValue;
			nextTrillCV = (float)nextNote / 12.f;
			lastTrillCV = lastCV;
		}
		else if (gateIn < 10.f && gateOn)
		{
			outputs[GATE_OUTPUT].setVoltage(10.f);
			gateOn = false;
		}
		else if (!gateOn)
		{
			outputs[GATE_OUTPUT].setVoltage(0.f);
			gateOn = false;
			skipNote = false;
		}

		outputs[CV_OUTPUT].setVoltage(lastCV);
	}
};

struct MelodygenWidget : ModuleWidget
{
	std::shared_ptr<Font> font;

	MelodygenWidget(Melodygen *module)
	{
		setModule(module);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/mfont.ttf"));
		setPanel(createPanel(asset::plugin(pluginInstance, "res/melodygen.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		ParamWidget *rangeKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 32.299953)), module, Melodygen::RANGE_PARAM);
		if (rangeKnob->getParamQuantity())
		{
			rangeKnob->getParamQuantity()->snapEnabled = true;
		}
		addParam(rangeKnob);

		ParamWidget *scaleKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(38.099998, 32.299953)), module, Melodygen::SCALE_PARAM);
		if (scaleKnob->getParamQuantity())
		{
			scaleKnob->getParamQuantity()->snapEnabled = true;
		}
		addParam(scaleKnob);

		ParamWidget *keyKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(61.200001, 32.299953)), module, Melodygen::KEY_PARAM);
		if (keyKnob->getParamQuantity())
		{
			keyKnob->getParamQuantity()->snapEnabled = true;
		}
		addParam(keyKnob);

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 60.299976)), module, Melodygen::DISJUNCT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.099998, 60.299976)), module, Melodygen::RESTPROB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.200001, 60.299976)), module, Melodygen::TRILLPROB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 88.29998)), module, Melodygen::REPEAT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.099998, 88.29998)), module, Melodygen::ROOTGRAVITY_PARANM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.200001, 88.29998)), module, Melodygen::TRILLRATE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15, 115.15134)), module, Melodygen::GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(46.920708, 115.15134)), module, Melodygen::GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(63.891338, 115.15134)), module, Melodygen::CV_OUTPUT));
	}
	void draw(const DrawArgs &args) override
	{
		// Draw the original content
		ModuleWidget::draw(args);

		// Load font from cache
		std::string fontPath = asset::plugin(pluginInstance, "res/mfont.ttf");
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		// Don't draw text if font failed to load
		if (font)
		{
			// Select font handle
			nvgFontFaceId(args.vg, font->handle);
			// Set font size and alignment
			nvgFontSize(args.vg, 10.0);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
			nvgFillColor(args.vg, nvgRGBA(0, 0, 153, 153)); // RGBA: Blue

			// Convert the scale index to its string representation
			if (module)
			{
				int scaleIndex = static_cast<int>(module->params[Melodygen::SCALE_PARAM].getValue());
				std::string scaleName = ScaleUtils::SCALE_NAMES[scaleIndex];

				// Convert the key index to its string representation
				int keyIndex = static_cast<int>(module->params[Melodygen::KEY_PARAM].getValue());
				std::string keyName = ScaleUtils::KEY_NAMES[keyIndex];

				// Draw the text at a position
				nvgText(args.vg, mm2px(38.099998), mm2px(41.5), scaleName.c_str(), NULL);
				nvgText(args.vg, mm2px(61.200001), mm2px(41.5), keyName.c_str(), NULL);
			}
		}
	}
};

Model *melodygenModel = createModel<Melodygen, MelodygenWidget>("melodygen");