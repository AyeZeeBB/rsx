#include <pch.h>

#include <core/mdl/smd.h>
#include <core/utils/textbuffer.h>

namespace smd
{
	void CSourceModelData::InitNode(const char* name, const int index, const int parent) const
	{
		Node& node = nodes[index];

		// [rika]: node has already been set
		if (node.name)
		{
			assertm(false, "should not be reinitalizing a node");
			return;
		}

		node.name = name;
		node.index = index;
		node.parent = parent;
	}

	void CSourceModelData::InitFrameBone(const int iframe, const int ibone, const Vector& pos, const RadianEuler& rot) const
	{
		assertm(iframe < numFrames, "frame out of range");
		assertm(ibone < numNodes, "node out of range");

		Frame* const frame = &frames[iframe];

		const int boneCount = static_cast<int>(frame->bones.size());

		if (ibone == boneCount)
		{
			frame->bones.emplace_back(ibone, pos, rot);

			return;
		}

		if (ibone < boneCount)
		{
			Bone& bone = frame->bones.at(ibone);

			bone.node = ibone;
			bone.pos = pos;
			bone.rot = rot;

			return;
		}

		assertm(false, "bones added out of order or not pre-sized");
	}

	// [rika]: this does not round floats to 6 decimal points as required by SMD, HOWEVER studiomdl supports scientific notation so we should be ok! 
	// [rika]: if this causes any weird issues in tthe future such as messed up skeletons we can change it
	void CSourceModelData::Write() const
	{
		std::filesystem::path outPath(exportPath);
		outPath.append(exportName);
		outPath.replace_extension(".smd");

		std::ofstream out(outPath, std::ios::out);

		out << "version 1\n";

		out << "nodes\n";
		for (size_t i = 0; i < numNodes; i++)
		{
			const Node& node = nodes[i];

			out << "\t" << node.index << " \"" << node.name << "\" " << node.parent << "\n";
		}
		out << "end\n";

		out << "skeleton\n";
		for (size_t iframe = 0; iframe < numFrames; iframe++)
		{
			const Frame& frame = frames[iframe];

			out << "\ttime " << iframe << "\n";

			for (const Bone& bone : frame.bones)
			{
				out << "\t\t" << bone.node << " ";
				out << bone.pos.x << " " << bone.pos.y << " " << bone.pos.z << " ";
				out << bone.rot.x << " " << bone.rot.y << " " << bone.rot.z << "\n";
			}
		}
		out << "end\n";

		if (triangles.size())
		{
			out << "triangles\n";
			for (size_t itriangle = 0; itriangle < triangles.size(); itriangle++)
			{
				const Triangle& triangle = triangles[itriangle];

				out << "" << triangle.material << "\n";

				for (int vertIdx = 0; vertIdx < 3; vertIdx++)
				{
					const Vertex& vert = triangle.vertices[vertIdx];

					out << "\t" << vert.bone[0] << " ";
					out << vert.position.x << " " << vert.position.y << " " << vert.position.z << " ";
					out << vert.normal.x << " " << vert.normal.y << " " << vert.normal.z << " ";
					out << vert.texcoords[0].x << " " << vert.texcoords[0].y << " " << vert.numBones << " ";

					for (int weightIdx = 0; weightIdx < vert.numBones; weightIdx++)
						out << vert.bone[weightIdx] << " " << vert.weight[weightIdx] << " ";

					out << "\n";
				}
			}
			out << "end\n";
		}

		out.close();
	}

	const bool CSourceModelData::Write(char* const buffer, const size_t size) const
	{
		if (!buffer || !size)
			return false;

		CTextBuffer textBuffer(buffer, size);
		textBuffer.SetTextStart();

		textBuffer.WriteString("version 1\n");
		textBuffer.WriteString("nodes\n");

		for (size_t i = 0; i < numNodes; i++)
		{
			const Node& node = nodes[i];
			textBuffer.WriteFormated("\t%i \"%s\" %i\n", node.index, node.name, node.parent);
		}
		textBuffer.WriteString("end\n");

		textBuffer.WriteString("skeleton\n");
		for (size_t iframe = 0; iframe < numFrames; iframe++)
		{
			const Frame& frame = frames[iframe];

			textBuffer.WriteFormated("\ttime %llu\n", iframe);

			for (const Bone& bone : frame.bones)
			{
				textBuffer.WriteFormated("\t\t%i %f %f %f %f %f %f\n", bone.node, bone.pos.x, bone.pos.y, bone.pos.z, bone.rot.x, bone.rot.y, bone.rot.z);
			}
		}
		textBuffer.WriteString("end\n");

		if (triangles.size())
		{
			textBuffer.WriteString("triangles\n");
			for (size_t itriangle = 0; itriangle < triangles.size(); itriangle++)
			{
				const Triangle& triangle = triangles[itriangle];

				textBuffer.WriteFormated("%s\n", triangle.material);

				for (int vertIdx = 0; vertIdx < 3; vertIdx++)
				{
					const Vertex& vert = triangle.vertices[vertIdx];

					// bone, pos xyz, normal xyz, texcoord xy, weight count 
					textBuffer.WriteFormated("\t%i %f %f %f %f %f %f %f %f %i", vert.bone[0],
						vert.position.x, vert.position.y, vert.position.z,
						vert.normal.x, vert.normal.y, vert.normal.z,
						vert.texcoords[0].x, vert.texcoords[0].y, vert.numBones);

					for (int weightIdx = 0; weightIdx < vert.numBones; weightIdx++)
						textBuffer.WriteFormated(" %i %f", vert.bone[weightIdx], vert.weight[weightIdx]);

					textBuffer.WriteCharacter('\n');
				}
			}
			textBuffer.WriteString("end\n");
		}

		std::filesystem::path outPath(exportPath);
		outPath.append(exportName);
		outPath.replace_extension(".smd");

#ifndef STREAMIO
		std::ofstream file(outPath);
		file.write(textBuffer.Text(), textBuffer.TextLength());
#else
		StreamIO file(outPath, eStreamIOMode::Write);
		file.write(textBuffer.Text(), textBuffer.TextLength());
#endif // !STREAMIO

		return true;
	}
}